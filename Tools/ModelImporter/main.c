#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <Render/Model.h>
#include <Runtime/Runtime.h>

#define BUFFER_PTR(acc) ((uint8_t *)acc->buffer_view->buffer->data + acc->buffer_view->offset + acc->offset)

#define NMESH_4_HEADER		0x0000344853454D4Ellu	// NMESH4
#define NMESH_FOOTER		0x004853454D444E45llu	// ENDMESH
#define NMESH_SEC_FOOTER	0x0054434553444E45llu	// ENDSECT
#define NMESH_VTX_ID		0x00585456u				// VTX
#define NMESH_IDX_ID		0x00584449u				// IDX
#define NMESH_MAT_ID		0x0054414Du				// MAT
#define NMESH_BONE_ID		0x454E4F42u				// BONE
#define NMESH_MESH_ID		0x4853454Du				// MESH

struct NMeshInfo
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	char material[256];
};

static inline void _convertMesh(const cgltf_mesh *mesh, const char *dir);
static inline void _saveMesh(const char *path);
static inline void _saveMaterial(const cgltf_material *mat, const char *dir, const char *path);
static inline void _saveAnimation(const cgltf_animation *anim, const char *path);
static inline void _saveImage(const cgltf_image *img, const char *path);

static enum IndexType _it;
static size_t _itSize;
static struct Vertex *_vertices;
static uint8_t *_indices;
static struct NMeshInfo *_meshes;
static uint32_t _vertexCount, _indexCount, _meshCount;

int
main(int argc, char *argv[])
{
	cgltf_options opt = { 0 };
	cgltf_data *gltf = NULL;

	const char *baseDir = argv[2];
	
	printf("Loading file...");

	if (cgltf_parse_file(&opt, argv[1], &gltf) != cgltf_result_success) {
		fprintf(stderr, "failed to parse file\n");
		return -1;
	}

	if (cgltf_load_buffers(&opt, gltf, "") != cgltf_result_success) {
		fprintf(stderr, "failed to load buffers\n");
		return -1;
	}

	printf("done.\nConverting meshes...");

	char pathBuff[4096];
	size_t vertexCount = 0, indexCount = 0, meshCount = 0;

	_it = IT_UINT_16;
	for (uint32_t i = 0; i < gltf->meshes_count; ++i) {
		for (uint32_t j = 0; j < gltf->meshes[i].primitives_count; ++j) {
			if (gltf->meshes[i].primitives[j].type != cgltf_primitive_type_triangles)
				continue;

			++meshCount;
			vertexCount += gltf->meshes[i].primitives[j].attributes[0].data->count;
			indexCount += gltf->meshes[i].primitives[j].indices->count;

			switch (gltf->meshes[i].primitives[j].indices->component_type) {
			case cgltf_component_type_r_16u:
				_it = _it < IT_UINT_16 ? IT_UINT_16 : _it;
			break;
			case cgltf_component_type_r_32u:
				_it = _it < IT_UINT_32 ? IT_UINT_32 : _it;
			break;
			default:
			break;
			}
		}
	}

	switch (_it) {
	case IT_UINT_16: _itSize = sizeof(uint16_t); break;
	case IT_UINT_32: _itSize = sizeof(uint32_t); break;
	}

	_vertices = calloc(vertexCount, sizeof(*_vertices));
	_indices = calloc(indexCount, _itSize);
	_meshes = calloc(meshCount, sizeof(*_meshes));

	for (uint32_t i = 0; i < gltf->meshes_count; ++i)
		_convertMesh(&gltf->meshes[i], baseDir);

	snprintf(pathBuff, sizeof(pathBuff), "Data/Models/%s.nmesh", argv[2]);
	_saveMesh(pathBuff);

	printf("done.\nConverting materials..."); fflush(stdout);

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		if (gltf->images[i].name)
			continue;

		gltf->images[i].name = calloc(256, sizeof(char));
		snprintf(gltf->images[i].name, 256, "%s_%d", baseDir, i);
	}

	for (uint32_t i = 0; i < gltf->materials_count; ++i) {
		snprintf(pathBuff, sizeof(pathBuff), "Data/Materials/%s/%s.mat", baseDir, gltf->materials[i].name);
		_saveMaterial(&gltf->materials[i], baseDir, pathBuff);
	}

	printf("done.\nConverting animations..."); fflush(stdout);

	for (uint32_t i = 0; i < gltf->animations_count; ++i) {
	}

	printf("done.\nConverting textures..."); fflush(stdout);

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		snprintf(pathBuff, sizeof(pathBuff), "Data/Textures/%s/%s.png", baseDir, gltf->images[i].name);
		_saveImage(&gltf->images[i], pathBuff);
	}

	printf("done.\n");
}

static inline void
_convertMesh(const cgltf_mesh *mesh, const char *dir)
{
	printf("\n");
	for (uint32_t i = 0; i < mesh->primitives_count; ++i) {
		const cgltf_primitive *prim = &mesh->primitives[i];
		
		if (prim->type != cgltf_primitive_type_triangles)
			continue;

		struct NMeshInfo *dstMesh = &_meshes[_meshCount++];

		snprintf(dstMesh->material, sizeof(dstMesh->material), "/Materials/%s/%s.mat",
					dir, prim->material ? prim->material->name : "Default");

		dstMesh->vertexOffset = _vertexCount;
		dstMesh->indexOffset = _indexCount;
		dstMesh->indexCount = (uint32_t)prim->indices->count;

		if (prim->indices->component_type == cgltf_component_type_r_32u) {
			assert(_it == IT_UINT_32);

			const uint32_t *idx = (const uint32_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < dstMesh->indexCount; ++j)
				((uint32_t *)_indices)[_indexCount++] = idx[j];
		} else if (prim->indices->component_type == cgltf_component_type_r_16u) {
			assert(_it >= IT_UINT_16);

			const uint16_t *idx = (const uint16_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < dstMesh->indexCount; ++j) {
				switch (_it) {
				case IT_UINT_16: ((uint16_t *)_indices)[_indexCount++] = idx[j]; break;
				case IT_UINT_32: ((uint32_t *)_indices)[_indexCount++] = idx[j]; break;
				}
			}
		} else if (prim->indices->component_type == cgltf_component_type_r_8u) {
			const uint8_t *idx = (const uint8_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < dstMesh->indexCount; ++j) {
				switch (_it) {
				case IT_UINT_16: ((uint16_t *)_indices)[_indexCount++] = idx[j]; break;
				case IT_UINT_32: ((uint32_t *)_indices)[_indexCount++] = idx[j]; break;
				}
			}
		}

		const float *pos = NULL, *norm = NULL, *tgt = NULL, *tc = NULL;
		for (uint32_t j = 0; j < prim->attributes_count; ++j) {
			const cgltf_attribute *attr = &prim->attributes[j];

			switch (attr->type) {
			case cgltf_attribute_type_position:
				pos = (const float *)BUFFER_PTR(attr->data);
				dstMesh->vertexCount = (uint32_t)attr->data->count;
			break;
			case cgltf_attribute_type_normal: norm = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_tangent: tgt = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_texcoord: tc = (const float *)BUFFER_PTR(attr->data); break;
			}
		}

		for (uint32_t j = 0; j < dstMesh->vertexCount; ++j) {
			struct Vertex *v = &_vertices[_vertexCount++];

			if (pos) {
				v->x = *pos++;
				v->y = *pos++;
				v->z = *pos++;
			}

			if (norm) {
				v->nx = *norm++;
				v->ny = *norm++;
				v->nz = *norm++;
			}

			if (tgt) {
				v->tx = *tgt++;
				v->ty = *tgt++;
				v->tz = *tgt++;
			}

			if (tc) {
				v->u = *tc++;
				v->v = *tc++;
			}
		}

		printf("\tvo %d, vc %d, io %d, ic %d, mat %s\n",
			dstMesh->vertexOffset, dstMesh->vertexCount,
			dstMesh->indexOffset, dstMesh->indexCount,
			dstMesh->material);
	}
}

static inline void
_saveMesh(const char *path)
{
	union {
		uint64_t guard;
		struct {
			uint32_t id;
			uint32_t size;
		};
	} a;

#define WRITE_SEC(i, v) a.id = i; a.size = v; fwrite(&a, sizeof(a), 1, fp)
#define WRITE_GUARD(x) a.guard = x; fwrite(&a, sizeof(a), 1, fp)

	FILE *fp = fopen(path, "wb");
	assert(fp);

	WRITE_GUARD(NMESH_4_HEADER);

	WRITE_SEC(NMESH_VTX_ID, sizeof(*_vertices) * _vertexCount);
	fwrite(_vertices, sizeof(*_vertices), _vertexCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_SEC(NMESH_IDX_ID, (uint32_t)_itSize * _indexCount);
	uint32_t type = (uint32_t)_it;
	fwrite(&type, sizeof(type), 1, fp);
	fwrite(_indices, _itSize, _indexCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	/*WRITE_SEC(NMESH_BONE_ID, sizeof(*_vertices) * _vertexCount);
	fwrite(_vertices, sizeof(*_vertices), _vertexCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);*/

	WRITE_SEC(NMESH_MESH_ID, _meshCount);
	fwrite(_meshes, sizeof(*_meshes), _meshCount, fp);
	WRITE_GUARD(NMESH_SEC_FOOTER);

	WRITE_GUARD(NMESH_FOOTER);

	fclose(fp);
}

static inline void
_saveMaterial(const cgltf_material *mat, const char *dir, const char *path)
{
	FILE *fp = fopen(path, "w");
	assert(fp);
	fprintf(fp, "{\n\t\"id\": \"NeMaterial\",\n\t\"version\": 1,\n\t\"name\": \"%s\",\n\t\"type\": \"Default\"", mat->name);

#define ARG_F(n, v) fprintf(fp, ",\n\t\"%s\": %.02f", n, v)
#define ARG_F3(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f\"", n, v[0], v[1], v[2])
#define ARG_F4(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f, %.02f\"", n, v[0], v[1], v[2], v[3])
#define ARG_IMG(n, v) fprintf(fp, ",\n\t\"%s\": \"/Textures/%s/%s.png\"", n, dir, v)

	ARG_F("AlphaCutoff", mat->alpha_cutoff);
	ARG_F3("EmissiveColor", mat->emissive_factor);

	if (mat->normal_texture.texture)
		ARG_IMG("NormalMap", mat->normal_texture.texture->image->name);

	if (mat->emissive_texture.texture)
		ARG_IMG("EmissiveMap", mat->emissive_texture.texture->image->name);

	if (mat->occlusion_texture.texture)
		ARG_IMG("OcclusionMap", mat->occlusion_texture.texture->image->name);

	if (mat->has_pbr_metallic_roughness) {
		ARG_F("Roughness", mat->pbr_metallic_roughness.roughness_factor);
		ARG_F("Metallic", mat->pbr_metallic_roughness.metallic_factor);
		ARG_F4("DiffuseColor", mat->pbr_metallic_roughness.base_color_factor);

		if (mat->pbr_metallic_roughness.base_color_texture.texture)
			ARG_IMG("DiffuseMap", mat->pbr_metallic_roughness.base_color_texture.texture->image->name);

		if (mat->pbr_metallic_roughness.metallic_roughness_texture.texture)
			ARG_IMG("MetallicRoughnessMap", mat->pbr_metallic_roughness.metallic_roughness_texture.texture->image->name);
	}

	if (mat->has_clearcoat) {
		ARG_F("ClearCoat", mat->clearcoat.clearcoat_factor);
		ARG_F("ClearCoatRoughness", mat->clearcoat.clearcoat_roughness_factor);

		if (mat->clearcoat.clearcoat_texture.texture)
			ARG_IMG("ClearCoatMap", mat->clearcoat.clearcoat_texture.texture->image->name);

		if (mat->clearcoat.clearcoat_normal_texture.texture)
			ARG_IMG("ClearCoatNormalMap", mat->clearcoat.clearcoat_normal_texture.texture->image->name);

		if (mat->clearcoat.clearcoat_roughness_texture.texture)
			ARG_IMG("ClearCoatRoughnessMap", mat->clearcoat.clearcoat_roughness_texture.texture->image->name);
	}

	if (mat->has_transmission) {
		ARG_F("Transmission", mat->transmission.transmission_factor);
		
		if (mat->transmission.transmission_texture.texture)
			ARG_IMG("TransmissionMap", mat->transmission.transmission_texture.texture->image->name);
	}

	ARG_F("SpecularWeight", mat->specular.specular_factor);

	fprintf(fp, "\n}\n");
	fclose(fp);
}

static inline void
_saveImage(const cgltf_image *img, const char *path)
{
	assert(img->buffer_view->buffer->data);

	int w, h, c;
	stbi_uc *imageData = stbi_load_from_memory((uint8_t *)img->buffer_view->buffer->data + img->buffer_view->offset,
												(int)img->buffer_view->buffer->size, &w, &h, &c, 4);
	assert(imageData);

	stbi_write_png(path, w, h, 4, imageData, 0);
}

