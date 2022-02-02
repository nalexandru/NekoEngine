#include <assert.h>

#include <Engine/IO.h>
#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Asset/NMesh.h>
#include <Editor/Asset/Import.h>
#include <System/PlatformDetect.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define BUFFER_PTR(acc) ((uint8_t *)acc->buffer_view->buffer->data + acc->buffer_view->offset + acc->offset)

static bool _MatchAsset(const char *path);
static bool _ImportAsset(const char *path);

static inline void _ConvertMesh(const cgltf_mesh *mesh, const char *name, struct NMesh *nm);
static inline void _SaveMaterial(const cgltf_material *mat, const char *name, const char *path);
//static inline void _SaveAnimation(const cgltf_animation *anim, const char *path);
static inline void _SaveImage(const cgltf_image *img, const char *path);

struct NeAssetImportHandler Ed_glTFImporter =
{
	.name = "glTF Importer",
	.Match = _MatchAsset,
	.Import = _ImportAsset
};

static bool
_MatchAsset(const char *path)
{
	return strstr(path, ".glb") != NULL;
}

static bool
_ImportAsset(const char *path)
{
	cgltf_options opt = { 0 };
	cgltf_data *gltf = NULL;
	struct NMesh nm = { 0 };
	size_t vertexCount = 0, indexCount = 0, meshCount = 0;

	char *baseDir = strdup(path);
	char *name = strrchr(baseDir, ED_DIR_SEPARATOR);
	*name++ = 0x0;

	char *ptr = strrchr(name, '.');
	if (ptr)
		*ptr = 0x0;

	EdGUI_ShowProgressDialog("Loading glTF...");

	if (cgltf_parse_file(&opt, path, &gltf) != cgltf_result_success) {
		EdGUI_MessageBox("FATAL", "Failed to parse glTF file");
		return false;
	}

	if (cgltf_load_buffers(&opt, gltf, "") != cgltf_result_success) {
		EdGUI_MessageBox("FATAL", "Failed to load glTF buffers");
		return false;
	}

	EdGUI_UpdateProgressDialog("Converting meshes...");

	char pathBuff[4096];

	nm.indexType = IT_UINT_16;
	for (uint32_t i = 0; i < gltf->meshes_count; ++i) {
		for (uint32_t j = 0; j < gltf->meshes[i].primitives_count; ++j) {
			if (gltf->meshes[i].primitives[j].type != cgltf_primitive_type_triangles)
				continue;

			++meshCount;
			vertexCount += gltf->meshes[i].primitives[j].attributes[0].data->count;
			indexCount += gltf->meshes[i].primitives[j].indices->count;

			switch (gltf->meshes[i].primitives[j].indices->component_type) {
			case cgltf_component_type_r_16u:
				nm.indexType = nm.indexType < IT_UINT_16 ? IT_UINT_16 : nm.indexType;
			break;
			case cgltf_component_type_r_32u:
				nm.indexType = nm.indexType < IT_UINT_32 ? IT_UINT_32 : nm.indexType;
			break;
			default:
			break;
			}
		}
	}

	switch (nm.indexType) {
	case IT_UINT_16: nm.indexSize = sizeof(uint16_t); break;
	case IT_UINT_32: nm.indexSize = sizeof(uint32_t); break;
	}

	nm.vertices = calloc(vertexCount, sizeof(*nm.vertices));
	nm.indices = calloc(indexCount, nm.indexSize);
	nm.meshes = calloc(meshCount, sizeof(*nm.meshes));

	for (uint32_t i = 0; i < gltf->meshes_count; ++i)
		_ConvertMesh(&gltf->meshes[i], name, &nm);

	snprintf(pathBuff, sizeof(pathBuff), "%s%cModels%c%s.nmesh", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);

	Asset_OptimizeNMesh(&nm);
	Asset_SaveNMesh(&nm, pathBuff);

	EdGUI_UpdateProgressDialog("Converting materials...");

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		if (gltf->images[i].name)
			continue;

		gltf->images[i].name = calloc(256, sizeof(char));
		snprintf(gltf->images[i].name, 256, "%s_%d", name, i);
	}

	if (gltf->materials_count) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cMaterials%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}

	for (uint32_t i = 0; i < gltf->materials_count; ++i) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cMaterials%c%s%c%s.mat", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, gltf->materials[i].name);
		_SaveMaterial(&gltf->materials[i], name, pathBuff);
	}

	EdGUI_UpdateProgressDialog("Converting animations...");

	for (uint32_t i = 0; i < gltf->animations_count; ++i) {
	}

	EdGUI_UpdateProgressDialog("Converting textures...");

	if (gltf->images_count) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s%c%s.png", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, gltf->images[i].name);
		_SaveImage(&gltf->images[i], pathBuff);
	}

	EdGUI_HideProgressDialog();

	cgltf_free(gltf);

	return true;
}

static inline void
_ConvertMesh(const cgltf_mesh *mesh, const char *name, struct NMesh *nm)
{
	for (uint32_t i = 0; i < mesh->primitives_count; ++i) {
		const cgltf_primitive *prim = &mesh->primitives[i];

		enum NePrimitiveType pt = PT_TRIANGLES;
		switch (prim->type) {
		case cgltf_primitive_type_triangles: pt = PT_TRIANGLES; break;
		case cgltf_primitive_type_points: pt = PT_POINTS; break;
		case cgltf_primitive_type_lines: pt = PT_LINES; break;
		default: continue;
		}

		struct NMeshSubmesh *dstMesh = &nm->meshes[nm->meshCount++];
		dstMesh->type = pt;

		if (prim->material)
			snprintf(dstMesh->material, sizeof(dstMesh->material), "/Materials/%s/%s.mat", name, prim->material->name);
		else
			snprintf(dstMesh->material, sizeof(dstMesh->material), "/Materials/Default.mat");

		dstMesh->vertexOffset = nm->vertexCount;
		dstMesh->indexOffset = nm->indexCount;
		dstMesh->indexCount = (uint32_t)prim->indices->count;

		if (prim->indices->component_type == cgltf_component_type_r_32u) {
			assert(nm->indexType == IT_UINT_32);

			const uint32_t *idx = (const uint32_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < dstMesh->indexCount; ++j)
				((uint32_t *)nm->indices)[nm->indexCount++] = idx[j];
		} else if (prim->indices->component_type == cgltf_component_type_r_16u) {
			assert(nm->indexType >= IT_UINT_16);

			const uint16_t *idx = (const uint16_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < dstMesh->indexCount; ++j) {
				switch (nm->indexType) {
				case IT_UINT_16: ((uint16_t *)nm->indices)[nm->indexCount++] = idx[j]; break;
				case IT_UINT_32: ((uint32_t *)nm->indices)[nm->indexCount++] = idx[j]; break;
				}
			}
		} else if (prim->indices->component_type == cgltf_component_type_r_8u) {
			const uint8_t *idx = (const uint8_t *)BUFFER_PTR(prim->indices);
			for (uint32_t j = 0; j < dstMesh->indexCount; ++j) {
				switch (nm->indexType) {
				case IT_UINT_16: ((uint16_t *)nm->indices)[nm->indexCount++] = idx[j]; break;
				case IT_UINT_32: ((uint32_t *)nm->indices)[nm->indexCount++] = idx[j]; break;
				}
			}
		}

		const float *pos = NULL, *norm = NULL, *tgt = NULL, *tc = NULL;
		const float *color = NULL, *joints = NULL, *weights = NULL;
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
			case cgltf_attribute_type_color: color = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_joints: joints = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_weights: weights = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_invalid: break;
			}
		}

		for (uint32_t j = 0; j < dstMesh->vertexCount; ++j) {
			struct NeVertex *v = &nm->vertices[nm->vertexCount++];

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

			if (color) {
				v->r = *color++;
				v->g = *color++;
				v->b = *color++;
				v->a = *color++;
			} else {
				v->r = 1.f; v->g = 1.f; v->b = 1.f; v->a = 1.f;
			}
		}
	}
}

static inline void
_SaveMaterial(const cgltf_material *mat, const char *name, const char *path)
{
	FILE *fp = fopen(path, "w");
	assert(fp);
	fprintf(fp, "{\n\t\"id\": \"NeMaterial\",\n\t\"version\": 1,\n\t\"name\": \"%s\",\n\t\"type\": \"Default\"", mat->name);

#define ARG_F(n, v) fprintf(fp, ",\n\t\"%s\": %.02f", n, v)
#define ARG_F3(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f\"", n, v[0], v[1], v[2])
#define ARG_F4(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f, %.02f\"", n, v[0], v[1], v[2], v[3])
#define ARG_IMG(n, v) fprintf(fp, ",\n\t\"%s\": \"/Textures/%s/%s.png\"", n, name, v)

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
_SaveImage(const cgltf_image *img, const char *path)
{
	assert(img->buffer_view->buffer->data);

	int w, h, c;
	stbi_uc *imageData = stbi_load_from_memory((uint8_t *)img->buffer_view->buffer->data + img->buffer_view->offset,
												(int)img->buffer_view->buffer->size, &w, &h, &c, 4);
	assert(imageData);

	stbi_write_png(path, w, h, 4, imageData, 0);
}
