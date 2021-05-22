#include <assert.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Runtime/Array.h>
#include <System/System.h>
#include <System/Memory.h>
#include <System/Log.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "stb_image.h"

struct Image
{
	uint64_t hash;
	int channels;
	enum TextureFormat format;
};

#define ADD_IMAGE(name, channels, fmt) {						\
	struct Image img = { Rt_HashString(name), channels, fmt };	\
	Rt_ArrayAdd(&images, &img);									\
}

static void *_gltfAlloc(void *user, cgltf_size size);
static void _gltfFree(void *user, void *ptr);
static cgltf_result _gltfRead(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data);
static void _gltfRelease(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, void* data);

#define BUFFER_PTR(acc) ((uint8_t *)acc->buffer_view->buffer->data + acc->buffer_view->offset + acc->offset)

static inline void _LoadMesh(const cgltf_mesh *mesh, struct Array *vertices, struct Array *indices, struct Array *meshes, struct Array *materials, enum IndexType it);
static inline Handle _CreateMaterial(const char *resName, const cgltf_material *mat);

bool
E_LoadglTFAsset(struct ResourceLoadInfo *li, struct Model *m)
{
	cgltf_options opt = { 0 };
	cgltf_data *gltf = NULL;
	struct Array vertices = { 0 }, indices = { 0 }, meshes = { 0 }, materials = { 0 }, images = { 0 };

	opt.memory.alloc = _gltfAlloc;
	opt.memory.free = _gltfFree;
	opt.file.read = _gltfRead;
	opt.file.release = _gltfRelease;
	opt.file.user_data = &li->stm;

	if (cgltf_parse_file(&opt, NULL, &gltf) != cgltf_result_success) {
		return false;
	}

	if (cgltf_load_buffers(&opt, gltf, "") != cgltf_result_success) {
		cgltf_free(gltf);
		return false;
	}

	enum IndexType it = IT_UINT_16;
	for (uint32_t i = 0; i < gltf->meshes_count; ++i) {
		meshes.size += gltf->meshes[i].primitives_count;

		for (uint32_t j = 0; j < gltf->meshes[i].primitives_count; ++j) {
			vertices.size += gltf->meshes[i].primitives[j].attributes[0].data->count;
			indices.size += gltf->meshes[i].primitives[j].indices->count;

			switch (gltf->meshes[i].primitives[j].indices->component_type) {
			case cgltf_component_type_r_16u:
				it = it < IT_UINT_16 ? IT_UINT_16 : it;
			break;
			case cgltf_component_type_r_32u:
				it = it < IT_UINT_32 ? IT_UINT_32 : it;
			break;
			default:
			break;
			}
		}
	}

	Rt_InitArray(&meshes, meshes.size, sizeof(struct Mesh), MH_Asset);
	Rt_InitArray(&vertices, vertices.size, sizeof(struct Vertex), MH_Asset);
	Rt_InitPtrArray(&materials, meshes.size, MH_Asset);
	Rt_InitArray(&images, 10, sizeof(struct Image), MH_Asset);

	size_t indexTypeSize = 1;
	switch (it) {
	case IT_UINT_16: indexTypeSize = sizeof(uint16_t); break;
	case IT_UINT_32: indexTypeSize = sizeof(uint32_t); break;
	}

	Rt_InitArray(&indices, indices.size, indexTypeSize, MH_Asset);

	for (uint32_t i = 0; i < gltf->meshes_count; ++i)
		_LoadMesh(&gltf->meshes[i], &vertices, &indices, &meshes, &materials, it);

	m->cpu.vertices = vertices.data;
	m->cpu.vertexSize = (uint32_t)Rt_ArrayByteSize(&vertices);

	m->cpu.indices = indices.data;
	m->cpu.indexSize = (uint32_t)Rt_ArrayByteSize(&indices);

	m->meshes = (struct Mesh *)meshes.data;
	m->meshCount = (uint32_t)meshes.count;

	for (uint32_t i = 0; i < gltf->materials_count; ++i) {
		const cgltf_material *mat = &gltf->materials[i];
		assert(mat->name);

		char *matResource = Sys_Alloc(4096, 1, MH_Transient);
		snprintf(matResource, 4096, "%s#%s.mat", li->path, mat->name);

		m->meshes->materialResource = E_LoadResource(matResource, RES_MATERIAL);
		if (m->meshes->materialResource == E_INVALID_HANDLE)
			m->meshes->materialResource = _CreateMaterial(matResource, mat);
	}

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		size_t j = 0;
		uint64_t hash = 0;
		int x = 0, y = 0, comp = 0;
		const cgltf_image *img = &gltf->images[i];
		struct Image *imgInfo = NULL, *iiData = (struct Image *)images.data;
		stbi_uc *imageData = NULL;
		struct TextureCreateInfo tci =
		{
			.desc =
			{
				.width = 0, .height = 0, .depth = 1,
				.type = TT_2D, .format = TF_R8G8B8A8_UNORM,
				.usage = TU_SAMPLED,
				.arrayLayers = 1, .mipLevels = 1,
				.gpuOptimalTiling = true,
				.memoryType = MT_GPU_LOCAL
			},
			.data = NULL,
			.dataSize = 0,
			.keepData = false
		};

		if (!img->buffer_view->buffer->data)
			continue;

		hash = Rt_HashString(img->name);
		for (j = 0; j < images.count; ++j) {
			if (iiData[j].hash != hash)
				continue;

			imgInfo = &iiData[j];
			break;
		}

		if (!imgInfo)
			continue;

		imageData = stbi_load_from_memory((uint8_t *)img->buffer_view->buffer->data + img->buffer_view->offset, (int)img->buffer_view->buffer->size, &x, &y, &comp, imgInfo->channels);

		if (!imageData) {
			continue;
		}

		tci.desc.width = x;
		tci.desc.height = y;
		tci.data = imageData;
		tci.dataSize = sizeof(stbi_uc) * x * y * imgInfo->channels;
		tci.desc.format = imgInfo->format;

		E_CreateResource(img->name, RES_TEXTURE, &tci);
		Sys_Free(tci.data);
	}

	cgltf_free(gltf);

	return true;
}

bool
E_LoadglTFTexture(const char *name, struct ResourceLoadInfo *li)
{
	return true;
}

void *
_gltfAlloc(void *user, cgltf_size size)
{
	// TODO: maximum size for transient allocation
	return Sys_Alloc(size, 1, MH_Transient);
}

void
_gltfFree(void *user, void *ptr)
{
	Sys_Free(ptr);
}

cgltf_result
_gltfRead(const struct cgltf_memory_options *memoryOptions, const struct cgltf_file_options *fileOptions, const char *path, cgltf_size *size, void **data)
{
	struct Stream *stm = fileOptions->user_data;
	char *fileData = NULL;
	cgltf_size readSize;

	cgltf_size fileSize = size ? *size : 0;

	if (!fileSize)
		fileSize = (cgltf_size)E_StreamLength(stm);

	fileData = (char*)Sys_Alloc(fileSize, 1, MH_Asset);
	if (!fileData)
		return cgltf_result_out_of_memory;

	readSize = (cgltf_size)E_ReadStream(stm, fileData, fileSize);
	if (readSize != fileSize) {
		Sys_Free(fileData);
		return cgltf_result_io_error;
	}

	if (size)
		*size = fileSize;

	if (data)
		*data = fileData;

	return cgltf_result_success;
}

void
_gltfRelease(const struct cgltf_memory_options *memoryOptions, const struct cgltf_file_options *fileOptions, void *data)
{
	Sys_Free(data);
}

void
_LoadMesh(const cgltf_mesh *mesh, struct Array *vertices, struct Array *indices, struct Array *meshes, struct Array *materials, enum IndexType it)
{
	float *positions = NULL, *normals = NULL, *tangents = NULL, *texcoords = NULL;

	for (uint32_t i = 0; i < mesh->primitives_count; ++i) {
		const cgltf_primitive *prim = &mesh->primitives[i];
		struct Mesh *dst = Rt_ArrayAllocate(meshes);
		wchar_t *name = NULL;

		if (prim->type != cgltf_primitive_type_triangles) {
			//
			continue;
		}

		name = NULL;
		if (prim->material) {
			name = Sys_Alloc(strlen(prim->material->name) + 1, sizeof(*name), MH_Asset);
			mbstowcs(name, prim->material->name, strlen(prim->material->name));
		} else {
			name = wcsdup(L"Default");
		}

		Rt_ArrayAddPtr(materials, name);

		dst->vertexOffset = (uint32_t)vertices->count;
		dst->indexOffset = (uint32_t)indices->count;

		dst->indexCount = (uint32_t)prim->indices->count;

		Rt_ResizeArray(indices, indices->count ? indices->count + prim->indices->count : prim->indices->count);

#define LOAD_IDX_32(x) *(uint32_t *)Rt_ArrayAllocate(indices) = x
#define LOAD_IDX_16(x) *(uint16_t *)Rt_ArrayAllocate(indices) = x

		if (prim->indices->component_type == cgltf_component_type_r_32u) {
			assert(it == IT_UINT_32);
			uint32_t *idx = (uint32_t *)BUFFER_PTR(prim->indices);
			if (Sys_BigEndian())
				for (uint32_t j = 0; j < prim->indices->count; ++j)
					LOAD_IDX_32(Sys_SwapUint32(idx[j]));
			else
				for (uint32_t j = 0; j < prim->indices->count; ++j)
					LOAD_IDX_32(idx[j]);
		} else if (prim->indices->component_type == cgltf_component_type_r_16u) {
			assert(it >= IT_UINT_16);
			uint16_t *u16idx = (uint16_t *)BUFFER_PTR(prim->indices);
			if (Sys_BigEndian())
				for (uint32_t j = 0; j < prim->indices->count; ++j)
					switch (it) {
					case IT_UINT_16: LOAD_IDX_16(Sys_SwapUint16(u16idx[j])); break;
					case IT_UINT_32: LOAD_IDX_32(Sys_SwapUint16(u16idx[j])); break;
					}
			else
				for (uint32_t j = 0; j < prim->indices->count; ++j)
					switch (it) {
					case IT_UINT_16: LOAD_IDX_16(u16idx[j]); break;
					case IT_UINT_32: LOAD_IDX_32(u16idx[j]); break;
					}
		} else if (prim->indices->component_type == cgltf_component_type_r_8u) {
			uint8_t *u8idx = (uint8_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < prim->indices->count; ++j) {
				switch (it) {
				case IT_UINT_16: LOAD_IDX_16(u8idx[j]); break;
				case IT_UINT_32: LOAD_IDX_32(u8idx[j]); break;
				}
			}
		}

		for (uint32_t j = 0; j < prim->attributes_count; ++j) {
			const cgltf_attribute *attrib = &prim->attributes[j];

			if (attrib->type == cgltf_attribute_type_position) {
				positions = (float *)BUFFER_PTR(attrib->data);
				dst->vertexCount = (uint32_t)attrib->data->count;
			} else if (attrib->type == cgltf_attribute_type_normal) {
				normals = (float *)BUFFER_PTR(attrib->data);
			} else if (attrib->type == cgltf_attribute_type_tangent) {
				tangents = (float *)BUFFER_PTR(attrib->data);
			} else if (attrib->type == cgltf_attribute_type_texcoord) {
				texcoords = (float *)BUFFER_PTR(attrib->data);
			}
		}

		Rt_ResizeArray(vertices, vertices->count ? vertices->count + dst->vertexCount : dst->vertexCount);

		if (Sys_BigEndian()) {
			for (uint32_t j = 0; j < dst->vertexCount; ++j) {
				struct Vertex *v = Rt_ArrayAllocate(vertices);

				if (positions) {
					v->x = Sys_SwapFloat(*positions++);
					v->y = Sys_SwapFloat(*positions++);
					v->z = Sys_SwapFloat(*positions++);
				}

				if (normals) {
					v->nx = Sys_SwapFloat(*normals++);
					v->ny = Sys_SwapFloat(*normals++);
					v->nz = Sys_SwapFloat(*normals++);
				}

				if (tangents) {
					v->tx = Sys_SwapFloat(*tangents++);
					v->ty = Sys_SwapFloat(*tangents++);
					v->tz = Sys_SwapFloat(*tangents++);
				}

				if (texcoords) {
					v->u = Sys_SwapFloat(*texcoords++);
					v->v = Sys_SwapFloat(*texcoords++);
				}
			}
		} else {
			for (uint32_t j = 0; j < dst->vertexCount; ++j) {
				struct Vertex *v = Rt_ArrayAllocate(vertices);

				if (positions) {
					v->x = *positions++;
					v->y = *positions++;
					v->z = *positions++;
				}

				if (normals) {
					v->nx = *normals++;
					v->ny = *normals++;
					v->nz = *normals++;
				}

				if (tangents) {
					v->tx = *tangents++;
					v->ty = *tangents++;
					v->tz = *tangents++;
				}

				if (texcoords) {
					v->u = *texcoords++;
					v->v = *texcoords++;
				}
			}
		}
	}
}

static inline Handle
_CreateMaterial(const char *resName, const cgltf_material *mat)
{
	struct MaterialResourceCreateInfo mrci = { false, { 0 }, "Default", NULL };
	const char **args = Sys_Alloc(sizeof(char *), 40, MH_Transient);
	mrci.args = (const char **)args;

	switch (mat->alpha_mode) {
	case cgltf_alpha_mode_opaque:
		mrci.alphaBlend = false;
	break;
	case cgltf_alpha_mode_mask:
	case cgltf_alpha_mode_blend:
		mrci.alphaBlend = true;
	break;
	}

#define ADD_ARG(x) assert(args < mrci.args + 20); *args++ = (void *)x

	ADD_ARG("AlphaCutoffF");
	ADD_ARG(&mat->alpha_cutoff);

	ADD_ARG("EmissiveColorF");
	ADD_ARG(&mat->emissive_factor);

	if (mat->normal_texture.texture) {
		ADD_ARG("NormalMap");
		ADD_ARG(mat->normal_texture.texture->image->name);
	}

	if (mat->emissive_texture.texture) {
		ADD_ARG("EmissiveMap");
		ADD_ARG(mat->emissive_texture.texture->image->name);
	}

	if (mat->occlusion_texture.texture) {
		ADD_ARG("OcclusionMap");
		ADD_ARG(mat->occlusion_texture.texture->image->name);
	}

	if (mat->has_pbr_metallic_roughness) {
		ADD_ARG("RoughnessF");
		ADD_ARG(&mat->pbr_metallic_roughness.roughness_factor);

		ADD_ARG("MetallicF");
		ADD_ARG(&mat->pbr_metallic_roughness.metallic_factor);

		ADD_ARG("DiffuseF");
		ADD_ARG(&mat->pbr_metallic_roughness.base_color_factor);

		if (mat->pbr_metallic_roughness.base_color_texture.texture) {
			ADD_ARG("DiffuseMap");
			ADD_ARG(mat->pbr_metallic_roughness.base_color_texture.texture->image->name);
		}

		if (mat->pbr_metallic_roughness.metallic_roughness_texture.texture) {
			ADD_ARG("MetallicRoughnessMap");
			ADD_ARG(mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image->name);
		}
	} else if (mat->has_pbr_specular_glossiness) {
	}

	if (mat->has_clearcoat) {
		if (mat->clearcoat.clearcoat_texture.texture) {
			ADD_ARG("ClearCoatMap");
			ADD_ARG(mat->clearcoat.clearcoat_texture.texture->image->name);
		}

		if (mat->clearcoat.clearcoat_normal_texture.texture) {
			ADD_ARG("ClearCoatNormalMap");
			ADD_ARG(mat->clearcoat.clearcoat_normal_texture.texture->image->name);
		}

		if (mat->clearcoat.clearcoat_roughness_texture.texture) {
			ADD_ARG("ClearCoatRoughnessMap");
			ADD_ARG(mat->clearcoat.clearcoat_roughness_texture.texture->image->name);
		}

		ADD_ARG("ClearCoatF");
		ADD_ARG(&mat->clearcoat.clearcoat_factor);

		ADD_ARG("ClearCoatRoughnessF");
		ADD_ARG(&mat->clearcoat.clearcoat_roughness_factor);
	}

	if (mat->has_transmission) {
		ADD_ARG("TransmissionMap");
		ADD_ARG(mat->transmission.transmission_texture.texture->image->name);

		ADD_ARG("TransmissionF");
		ADD_ARG(&mat->transmission.transmission_factor);
	}

	strncpy(mrci.name, mat->name, sizeof(mrci.name));

	return E_CreateResource(resName, RES_MATERIAL, &mrci);
}
