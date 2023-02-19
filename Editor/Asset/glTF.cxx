#include <assert.h>

#include <Math/Math.h>
#include <Asset/NAnim.h>
#include <Asset/NMesh.h>
#include <Engine/IO.h>
#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Asset/Import.h>
#include <System/PlatformDetect.h>

struct Matrix
{
	float data[16];
};

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "Editor/Asset/Asset.h"

#define BUFFER_PTR(acc) ((uint8_t *)acc->buffer_view->buffer->data + acc->buffer_view->offset + acc->offset)

ED_ASSET_IMPORTER(glTF);

static inline void _ConvertMesh(const cgltf_mesh *mesh, const char *name, struct NMesh *nm);
static inline void _ConvertNode(const cgltf_node *node, struct NeArray *nodes, int32_t parentId);
static inline void _SaveMaterial(const cgltf_material *mat, const char *name, const char *path);
static inline void _SaveAnimation(const cgltf_animation *anim, const char *path);
static inline void _SaveImage(const cgltf_image *img, const char *path);

static bool
_MatchAsset(const char *path)
{
	return strstr(path, ".glb") != NULL || strstr(path, ".gltf") != NULL;
}

static bool
_ImportAsset(const char *path)
{
	cgltf_options opt = {};
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
		EdGUI_MessageBox("Error", "Failed to parse glTF file", MB_Error);
		free(baseDir);
		return false;
	}

	if (cgltf_load_buffers(&opt, gltf, "") != cgltf_result_success) {
		EdGUI_MessageBox("Error", "Failed to load glTF buffers", MB_Error);
		free(baseDir);
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

	nm.vertices = (struct NeVertex *)calloc(vertexCount, sizeof(*nm.vertices));
	nm.indices = (uint8_t *)calloc(indexCount, nm.indexSize);
	nm.meshes = (struct NMeshSubmesh *)calloc(meshCount, sizeof(*nm.meshes));

	for (uint32_t i = 0; i < gltf->meshes_count; ++i)
		_ConvertMesh(&gltf->meshes[i], name, &nm);

	snprintf(pathBuff, sizeof(pathBuff), "%s%cModels%c%s.nmesh", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);

	//Asset_OptimizeNMesh(&nm);
	Asset_SaveNMesh(&nm, pathBuff);

	EdGUI_UpdateProgressDialog("Converting materials...");

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		if (gltf->images[i].name)
			continue;

		gltf->images[i].name = (char *)calloc(256, sizeof(char));
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
	
	if (gltf->nodes_count) {
		struct NeArray nodes;
		Rt_InitArray(&nodes, 10, sizeof(struct NMeshNode), MH_Editor);

		_ConvertNode(&gltf->nodes[0], &nodes, -1);

		nm.nodes = (struct NMeshNode *)nodes.data;
		nm.nodeCount = (uint32_t)nodes.count;
	}

	if (gltf->skins_count) {
		cgltf_skin *skin = gltf->skins;
		struct Matrix *inverseBindMatrices = (struct Matrix *)BUFFER_PTR(skin->inverse_bind_matrices);

		for (int32_t i = 0; i < skin->joints_count; ++i) {
			cgltf_node *joint = skin->joints[i];

			
		}
	}

	if (gltf->animations_count) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cAnimations%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}

	for (uint32_t i = 0; i < gltf->animations_count; ++i) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cAnimations%c%s%c%s.png", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, gltf->animations[i].name);
		_SaveAnimation(&gltf->animations[i], pathBuff);
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
	free(baseDir);

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
		dstMesh->primitiveType = pt;

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
		const float *color = NULL, *weights = NULL;
		const int32_t *joints = NULL;
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
			case cgltf_attribute_type_joints: joints = (const int32_t *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_weights: weights = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_invalid: break;
			default: break;
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

			if (!weights || !joints)
				continue;

			struct NeVertexWeight *vw = &nm->vertexWeights[nm->vertexWeightCount++];

			vw->j0 = *joints++;
			vw->j1 = *joints++;
			vw->j2 = *joints++;
			vw->j3 = *joints++;

			vw->w0 = *weights++;
			vw->w1 = *weights++;
			vw->w2 = *weights++;
			vw->w3 = *weights++;
		}
	}
}

static inline void
_ConvertNode(const cgltf_node *node, struct NeArray *nodes, int32_t parentId)
{
	int32_t id = (int32_t)nodes->count;
	struct NMeshNode *nmNode = (struct NMeshNode *)Rt_ArrayAllocate(nodes);

	if (node->name && strlen(node->name))
		snprintf(nmNode->name, sizeof(nmNode->name), "%s", node->name);

	XMMATRIX mat = XMMatrixIdentity();

	if (node->has_matrix) {
		mat = XMLoadFloat4x4((XMFLOAT4X4 *)node->matrix);
	} else {
		if (node->has_scale)
			mat = XMMatrixMultiply(mat, XMMatrixScaling(node->scale[0], node->scale[1], node->scale[2]));

		if (node->has_rotation)
			mat = XMMatrixMultiply(mat, XMMatrixRotationQuaternion(XMLoadFloat4((XMFLOAT4 *)node->rotation)));

		if (node->has_translation)
			mat = XMMatrixMultiply(mat, XMMatrixTranslation(node->translation[0], node->translation[1], node->translation[2]));
	}

	XMStoreFloat4x4((XMFLOAT4X4 *)nmNode->transform, mat);

	if (node->children_count > 15) {
		// blyat
	}

	for (uint32_t j = 0; j < node->children_count; ++j)
		_ConvertNode(node->children[j], nodes, id);
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
_SaveAnimation(const cgltf_animation *anim, const char *path)
{
	struct NAnim na;

	na.channelCount = (uint32_t)anim->channels_count;

	for (cgltf_size i = 0; i < anim->channels_count; ++i) {
		const cgltf_animation_channel *ch = &anim->channels[i];
		struct NAnimChannel nch;
		
		snprintf(nch.name, sizeof(nch.name), "%s", ch->target_node->name);

		switch (ch->target_path) {
		case cgltf_animation_path_type_translation: {
			
		} break;
		case cgltf_animation_path_type_rotation:
		break;
		case cgltf_animation_path_type_scale:
		break;
		default:
		break;
		}
	}

	Asset_SaveNAnim(&na, path);
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

/* NekoEditor
 *
 * glTF.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
