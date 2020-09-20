#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Render/Texture.h>
#include <Render/Material.h>
#include <Runtime/Array.h>
#include <System/System.h>
#include <System/Memory.h>
#include <System/Log.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "stb_image.h"

static void *_gltfAlloc(void *user, cgltf_size size);
static void _gltfFree(void *user, void *ptr);
static cgltf_result _gltfRead(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data);
static void _gltfRelease(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, void* data);

#define BUFFER_PTR(acc) ((uint8_t *)acc->buffer_view->buffer->data + acc->buffer_view->offset + acc->offset)

static inline void _LoadMesh(const cgltf_mesh *mesh, Array *vertices, Array *indices, Array *meshes, Array *materials);

bool
E_LoadglTFAsset(const char *baseDir, struct Stream *stm, struct Model *m)
{
	cgltf_options opt = { 0 };
	cgltf_data *gltf = NULL;
	Array vertices = { 0 }, indices = { 0 }, meshes = { 0 }, materials = { 0 };
	uint32_t i = 0, j = 0;

	opt.memory.alloc = _gltfAlloc;
	opt.memory.free = _gltfFree;
	opt.file.read = _gltfRead;
	opt.file.release = _gltfRelease;
	opt.file.user_data = stm;

	if (cgltf_parse_file(&opt, NULL, &gltf) != cgltf_result_success) {
		return false;
	}

	if (cgltf_load_buffers(&opt, gltf, "") != cgltf_result_success) {
		cgltf_free(gltf);
		return false;
	}

	for (i = 0; i < gltf->meshes_count; ++i) {
		meshes.size += gltf->meshes[i].primitives_count;

		for (j = 0; j < gltf->meshes[i].primitives_count; ++j) {
			vertices.size += gltf->meshes[i].primitives[j].attributes[0].data->count;
			indices.size += gltf->meshes[i].primitives[j].indices->count;
		}
	}

	Rt_InitArray(&meshes, meshes.size, sizeof(struct Mesh));
	Rt_InitArray(&vertices, vertices.size, sizeof(struct Vertex));
	Rt_InitArray(&indices, indices.size, sizeof(uint32_t));
	Rt_InitPtrArray(&materials, meshes.size);

	for (i = 0; i < gltf->meshes_count; ++i)
		_LoadMesh(&gltf->meshes[i], &vertices, &indices, &meshes, &materials);

	m->vertices = (struct Vertex *)vertices.data;
	m->numVertices = (uint32_t)vertices.count;

	m->indices = (uint32_t *)indices.data;
	m->numIndices = (uint32_t)indices.count;

	m->meshes = (struct Mesh *)meshes.data;
	m->numMeshes = (uint32_t)meshes.count;
	m->materialNames = (wchar_t **)materials.data;

	for (i = 0; i < gltf->images_count; ++i) {
		int x = 0, y = 0, comp = 0;
		const cgltf_image *img = &gltf->images[i];
		stbi_uc *imageData = NULL;
		struct TextureCreateInfo tci =
		{
			0, 0, 1,
			TT_2D,
			TF_R8G8B8A8_UNORM,
			NULL,
			0,
			false
		};

		if (img->buffer_view->buffer->data)
			imageData = stbi_load_from_memory((uint8_t *)img->buffer_view->buffer->data + img->buffer_view->offset, (int)img->buffer_view->buffer->size, &x, &y, &comp, 4);

		if (!imageData) {
			continue;
		}

		tci.width = x;
		tci.height = y;
		tci.data = imageData;
		tci.dataSize = sizeof(stbi_uc) * x * y * 4;

		/*if (comp == 3) {
			tci.dataSize = sizeof(stbi_uc) * x * y * 4;
			tci.data = calloc(1, tci.dataSize);

			int k = 0, l = 0;
			for (int j = 0; j < x * y; ++j) {
				((uint8_t *)tci.data)[k++] = data[l++]; l++; l++; l++;
				((uint8_t *)tci.data)[k++] = 0x00;//data[l++];
				((uint8_t *)tci.data)[k++] = 0x00;//data[l++];
				((uint8_t *)tci.data)[k++] = 0xFF;
			}

			free(data);
		} else if (comp == 2) {
			tci.format = TF_R8G8_UNORM;
		} else if (comp == 1) {
			tci.format = TF_R8_UNORM;
		}*/

		E_CreateResource(img->name, RES_TEXTURE, &tci);
	}

	for (i = 0; i < gltf->materials_count; ++i) {
		struct MaterialProperties props;
		const cgltf_material *mat = &gltf->materials[i];
		const char *textures[RE_MAX_TEXTURES] = { 0 };
		wchar_t *name = NULL;

		switch (mat->alpha_mode) {
		case cgltf_alpha_mode_opaque: props.alphaMode = ALPHA_MODE_OPAQUE; break;
		case cgltf_alpha_mode_mask: props.alphaMode = ALPHA_MODE_MASK; break;
		case cgltf_alpha_mode_blend: props.alphaMode = ALPHA_MODE_BLEND; break;
		}

		props.alphaCutoff = mat->alpha_cutoff;
		memcpy(&props.emissive, mat->emissive_factor, sizeof(props.emissive));

		if (mat->normal_texture.texture)
			textures[MAP_NORMAL] = mat->normal_texture.texture->image->name;

		if (mat->emissive_texture.texture)
			textures[MAP_EMISSIVE] = mat->emissive_texture.texture->image->name;

		if (mat->occlusion_texture.texture)
			textures[MAP_AO] = mat->occlusion_texture.texture->image->name;

		if (mat->has_pbr_metallic_roughness) {
			props.roughness = mat->pbr_metallic_roughness.roughness_factor;
			props.metallic = mat->pbr_metallic_roughness.metallic_factor;

			memcpy(&props.color.a, mat->pbr_metallic_roughness.base_color_factor, sizeof(props.color));
			if (mat->pbr_metallic_roughness.base_color_texture.texture)
				textures[MAP_DIFFUSE] = mat->pbr_metallic_roughness.base_color_texture.texture->image->name;
			if (mat->pbr_metallic_roughness.metallic_roughness_texture.texture)
				textures[MAP_METALLIC_ROUGHNESS] = mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image->name;
		} else if (mat->has_pbr_specular_glossiness) {
		}

		if (mat->has_clearcoat) {
			if (mat->clearcoat.clearcoat_texture.texture)
				textures[MAP_CLEARCOAT] = mat->clearcoat.clearcoat_texture.texture->image->name;

			if (mat->clearcoat.clearcoat_normal_texture.texture)
				textures[MAP_CLEARCOAT_NORMAL] = mat->clearcoat.clearcoat_normal_texture.texture->image->name;

			if (mat->clearcoat.clearcoat_roughness_texture.texture)
				textures[MAP_CLEARCOAT_ROUGHNESS] = mat->clearcoat.clearcoat_roughness_texture.texture->image->name;

			props.clearcoat = mat->clearcoat.clearcoat_factor;
			props.clearcoatRoughness = mat->clearcoat.clearcoat_roughness_factor;
		}

		if (mat->has_transmission) {
			if (mat->transmission.transmission_texture.texture)
				textures[MAP_TRANSMISSION] = mat->transmission.transmission_texture.texture->image->name;

			props.transmission = mat->transmission.transmission_factor;
		}

		name = Sys_Alloc(sizeof(wchar_t), strlen(mat->name), MH_Transient);
		mbstowcs(name, mat->name, strlen(mat->name));

		Re_CreateMaterial(name, L"Default", &props, textures); 
	}

	cgltf_free(gltf);

	return true;
}

void *
_gltfAlloc(void *user, cgltf_size size)
{
	return Sys_Alloc(size, 1, MH_Transient);
}

void
_gltfFree(void *user, void *ptr)
{
	// do nothing
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

	fileData = (char*)Sys_Alloc(fileSize, 1, MH_Persistent);
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
_LoadMesh(const cgltf_mesh *mesh, Array *vertices, Array *indices, Array *meshes, Array *materials)
{
	float *positions = NULL, *normals = NULL, *tangents = NULL, *texcoords = NULL;
	uint32_t i, j;

	for (i = 0; i < mesh->primitives_count; ++i) {
		const cgltf_primitive *prim = &mesh->primitives[i];
		struct Mesh *dst = Rt_ArrayAllocate(meshes);
		wchar_t *name = NULL;

		if (prim->type != cgltf_primitive_type_triangles) {
			//
			continue;
		}

		name = NULL;
		if (prim->material) {
			name = calloc(strlen(prim->material->name) + 1, sizeof(*name));
			mbstowcs(name, prim->material->name, strlen(prim->material->name));
		} else {
			name = wcsdup(L"Default");
		}

		Rt_ArrayAddPtr(materials, name);

		dst->firstVertex = (uint32_t)vertices->count;
		dst->firstIndex = (uint32_t)indices->count;

		dst->indexCount = (uint32_t)prim->indices->count;

		Rt_ResizeArray(indices, indices->count ? indices->count + prim->indices->count : prim->indices->count);

		if (prim->indices->component_type == cgltf_component_type_r_32u) {
			uint32_t *idx = (uint32_t *)BUFFER_PTR(prim->indices);
			if (Sys_BigEndian())
				for (j = 0; j < prim->indices->count; ++j)
					*(uint32_t *)Rt_ArrayAllocate(indices) = Sys_SwapUint32(idx[j]);
			else
				for (j = 0; j < prim->indices->count; ++j)
					*(uint32_t *)Rt_ArrayAllocate(indices) = idx[j];
		} else if (prim->indices->component_type == cgltf_component_type_r_16u) {
			uint16_t *u16idx = (uint16_t *)BUFFER_PTR(prim->indices);
			if (Sys_BigEndian())
				for (j = 0; j < prim->indices->count; ++j)
					*(uint32_t *)Rt_ArrayAllocate(indices) = Sys_SwapUint16(u16idx[j]);
			else
				for (j = 0; j < prim->indices->count; ++j)
					*(uint32_t *)Rt_ArrayAllocate(indices) = u16idx[j];
		} else if (prim->indices->component_type == cgltf_component_type_r_8u) {
			uint8_t *u8idx = (uint8_t *)BUFFER_PTR(prim->indices);
			for (j = 0; j < prim->indices->count; ++j)
				*(uint32_t *)Rt_ArrayAllocate(indices) = u8idx[j];
		}

		for (j = 0; j < prim->attributes_count; ++j) {
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
			for (j = 0; j < dst->vertexCount; ++j) {
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
			for (j = 0; j < dst->vertexCount; ++j) {
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

