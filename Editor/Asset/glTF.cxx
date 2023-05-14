#include <assert.h>

#include <Math/Math.h>
#include <Asset/NAnim.h>
#include <Asset/NMesh.h>
#include <Engine/IO.h>
#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>
#include <System/PlatformDetect.h>
#include <System/System.h>
#include <System/Log.h>

#define GLTF_MOD	"glTFImport"

struct Matrix
{
	float data[16];
};

struct Vec3
{
	float x, y, z;
};

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

#define BUFFER_PTR(acc) ((uint8_t *)acc->buffer_view->buffer->data + acc->buffer_view->offset + acc->offset)

ED_ASSET_IMPORTER(glTF);

static inline void ConvertMesh(const cgltf_mesh *mesh, const char *name, struct NMesh *nm);
static inline void SaveMaterial(const cgltf_material *mat, const char *name, const char *path);
static inline void SaveAnimation(const cgltf_animation *anim, const char *name, const char *path);
static int32_t ChnCmpFunc(const struct NAnimChannel *chn, const char *name);

static bool
glTF_MatchAsset(const char *path)
{
	return strstr(path, ".glb") != NULL || strstr(path, ".gltf") != NULL;
}

static bool
glTF_ImportAsset(const char *path, const struct NeAssetImportOptions *options)
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

	char nameBuff[512], pathBuff[4096];

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

	nm.vertices = (struct NeVertex *)Sys_Alloc(sizeof(*nm.vertices), vertexCount, MH_Editor);
	nm.indices = (uint8_t *)Sys_Alloc(nm.indexSize, indexCount, MH_Editor);
	nm.meshes = (struct NMeshSubmesh *)Sys_Alloc(sizeof(*nm.meshes), meshCount, MH_Editor);

	if (gltf->skins_count)
		nm.vertexWeights = (struct NeVertexWeight *) Sys_Alloc(sizeof(*nm.vertexWeights), vertexCount, MH_Editor);

	for (uint32_t i = 0; i < gltf->meshes_count; ++i)
		ConvertMesh(&gltf->meshes[i], name, &nm);

	snprintf(pathBuff, sizeof(pathBuff), "%s%cModels%c%s.nmesh", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);

	if (gltf->skins_count) {
		cgltf_skin *skin = gltf->skins;

		{ // Nodes
			nm.nodes = (NMeshNode *)Sys_Alloc(sizeof(*nm.nodes), gltf->nodes_count, MH_Editor);
			nm.nodeCount = (uint32_t)gltf->nodes_count;

			for (cgltf_size i = 0; i < gltf->nodes_count; ++i) {
				cgltf_node *n = &gltf->nodes[i];
				strlcpy(nm.nodes[i].name, n->name, sizeof(nm.nodes[i].name));

				if (n->skin) {
					XMMATRIX git = XMLoadFloat4x4((XMFLOAT4X4 *)n->matrix);
					cgltf_node *p = n->parent;
					while (p) {
						git = XMMatrixMultiply(git, XMLoadFloat4x4((XMFLOAT4X4 *)p->matrix));
						p = p->parent;
					}
					M_Store(&nm.inverseTransform, XMMatrixInverse(NULL, git));
				}

				XMMATRIX mat = XMMatrixIdentity();

				if (n->has_matrix) {
					mat = XMLoadFloat4x4((XMFLOAT4X4 *)n->matrix);
				} else {
					if (n->has_scale)
						mat = XMMatrixMultiply(mat, XMMatrixScaling(n->scale[0], n->scale[1], n->scale[2]));

					if (n->has_rotation)
						mat = XMMatrixMultiply(mat, XMMatrixRotationQuaternion(XMLoadFloat4((XMFLOAT4 *)n->rotation)));

					if (n->has_translation)
						mat = XMMatrixMultiply(mat, XMMatrixTranslation(n->translation[0], n->translation[1], n->translation[2]));
				}

				XMStoreFloat4x4((XMFLOAT4X4 *)nm.nodes[i].xform, mat);

				if (!n->parent) {
					nm.nodes[i].parentId = -1;
					continue;
				}

				for (uint32_t j = 0; j < gltf->nodes_count; ++j) {
					if (n->parent != &gltf->nodes[j])
						continue;

					nm.nodes[i].parentId = j;
					break;
				}
			}
		}

		{ // Inverse Bind Matrices
			const size_t invMatSize = sizeof(*nm.inverseBindMatrices) * skin->inverse_bind_matrices->count;
			struct Matrix *inverseBindMatrices = (struct Matrix *)BUFFER_PTR(skin->inverse_bind_matrices);

			nm.invMatCount = (uint32_t)skin->inverse_bind_matrices->count;
			nm.inverseBindMatrices = (NeMatrix *)Sys_Alloc(invMatSize, 1, MH_Editor);
			memcpy(nm.inverseBindMatrices, inverseBindMatrices, invMatSize);
		}

		{ // Joints
			nm.joints = (uint32_t *)Sys_Alloc(sizeof(*nm.joints), skin->joints_count, MH_Editor);
			nm.jointCount = (uint32_t)skin->joints_count;

			for (uint32_t i = 0; i < skin->joints_count; ++i) {
				cgltf_node *joint = skin->joints[i];

				for (uint32_t j = 0; j < gltf->nodes_count; ++j) {
					if (joint != &gltf->nodes[j])
						continue;

					nm.joints[i] = j;
					break;
				}
			}
		}
	}

	EdAsset_OptimizeNMesh(&nm);
	if (options->inMemory)
		memcpy(options->dst, &nm, sizeof(nm));
	else
		EdAsset_SaveNMesh(&nm, pathBuff);

	if (options->onlyGeometry)
		goto exit;

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
		SaveMaterial(&gltf->materials[i], name, pathBuff);
	}

	EdGUI_UpdateProgressDialog("Converting animations...");

	if (gltf->animations_count) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cAnimations%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}

	for (uint32_t i = 0; i < gltf->animations_count; ++i) {
		if (!gltf->animations[i].name)
			snprintf(nameBuff, sizeof(nameBuff), "unnamed_%d", i);
		else
			snprintf(nameBuff, sizeof(nameBuff), "%s", gltf->animations[i].name);

		snprintf(pathBuff, sizeof(pathBuff), "%s%cAnimations%c%s%c%s.nanim", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, nameBuff);
		SaveAnimation(&gltf->animations[i], nameBuff, pathBuff);
	}

	EdGUI_UpdateProgressDialog("Converting morhps...");


	EdGUI_UpdateProgressDialog("Converting textures...");

	if (gltf->images_count) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}

	for (uint32_t i = 0; i < gltf->images_count; ++i) {
		if (!gltf->images[i].buffer_view || !gltf->images[i].buffer_view->buffer || !gltf->images[i].buffer_view->buffer->data)
			continue;

		snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s%c%s.png", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, gltf->images[i].name);
		EdAsset_ConvertToPNG(
			(uint8_t *) gltf->images[i].buffer_view->buffer->data + gltf->images[i].buffer_view->offset,
			(int) gltf->images[i].buffer_view->buffer->size,
			path
		);
	}

exit:
	EdGUI_HideProgressDialog();

	if (!options->inMemory)
		EdAsset_FreeNMesh(&nm);

	cgltf_free(gltf);
	free(baseDir);

	return true;
}

static inline void
ConvertMesh(const cgltf_mesh *mesh, const char *name, struct NMesh *nm)
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
		union {
			const uint8_t *u8joints;
			const uint16_t *u16joints;
		} joints = { NULL };
		cgltf_component_type jointType = cgltf_component_type_r_16u;
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
			case cgltf_attribute_type_weights: weights = (const float *)BUFFER_PTR(attr->data); break;
			case cgltf_attribute_type_joints:
				jointType = attr->data->component_type;
				joints.u8joints = (const uint8_t *)BUFFER_PTR(attr->data);
			break;
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

			if (!weights || !joints.u8joints)
				continue;

			struct NeVertexWeight *vw = &nm->vertexWeights[nm->vertexWeightCount++];
			switch (jointType) {
				case cgltf_component_type_r_8u:
					vw->j0 = *joints.u8joints++;
					vw->j1 = *joints.u8joints++;
					vw->j2 = *joints.u8joints++;
					vw->j3 = *joints.u8joints++;
				break;
				case cgltf_component_type_r_16u:
					vw->j0 = *joints.u16joints++;
					vw->j1 = *joints.u16joints++;
					vw->j2 = *joints.u16joints++;
					vw->j3 = *joints.u16joints++;
				break;
				default:
					continue;
			}

			vw->w0 = *weights++;
			vw->w1 = *weights++;
			vw->w2 = *weights++;
			vw->w3 = *weights++;
		}
	}
}

static inline void
SaveMaterial(const cgltf_material *mat, const char *name, const char *path)
{
	FILE *fp = fopen(path, "w");
	assert(fp);
	fprintf(fp, "{\n\t\"id\": \"NeMaterial\",\n\t\"version\": 1,\n\t\"name\": \"%s\",\n\t\"type\": \"Default\"", mat->name);

#define ARG_F(n, v) fprintf(fp, ",\n\t\"%s\": %.02f", (n), (v))
#define ARG_F3(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f\"", (n), (v)[0], (v)[1], (v)[2])
#define ARG_F4(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f, %.02f\"", (n), (v)[0], (v)[1], (v)[2], (v)[3])
#define ARG_IMG(n, v) fprintf(fp, ",\n\t\"%s\": \"/Textures/%s/%s.png\"", (n), name, (v))

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

	if (mat->has_specular)
		ARG_F("SpecularWeight", mat->specular.specular_factor);

	if (mat->has_ior)
		ARG_F("IOR", mat->ior.ior);

#undef ARG_F
#undef ARG_C
#undef ARG_IMG

	fprintf(fp, "\n}\n");
	fclose(fp);
}

static inline void
SaveAnimation(const cgltf_animation *anim, const char *name, const char *path)
{
	struct NAnim na = { .channelCount = 0 };
	strlcpy(na.info.name, name, sizeof(na.info.name));

	struct NeArray channels;
	Rt_InitArray(&channels, anim->channels_count / 3, sizeof(struct NAnimChannel), MH_Editor);

	for (cgltf_size i = 0; i < anim->channels_count; ++i) {
		const cgltf_animation_channel *ch = &anim->channels[i];
		struct NAnimChannel *nch = (struct NAnimChannel *)Rt_ArrayFind(&channels, ch->target_node->name, (RtCmpFunc)ChnCmpFunc);
		if (!nch) {
			nch = (struct NAnimChannel *)Rt_ArrayAllocate(&channels);
			strlcpy(nch->info.name, ch->target_node->name, sizeof(nch->info.name));
		}

		switch (ch->sampler->interpolation) {
		case cgltf_interpolation_type_linear: nch->info.interpolation = NANIM_INTERP_LINEAR; break;
		case cgltf_interpolation_type_step: nch->info.interpolation = NANIM_INTERP_STEP; break;
		case cgltf_interpolation_type_cubic_spline: nch->info.interpolation = NANIM_INTERP_CUBIC_SPLINE; break;
		}

		const float *time = (const float *)BUFFER_PTR(ch->sampler->input);
		for (cgltf_size j = 0; j < ch->sampler->input->count; ++j)
			na.info.duration = M_Max(na.info.duration, time[j]);

		switch (ch->target_path) {
		case cgltf_animation_path_type_translation: {
			struct NeArray keys;
			Rt_InitArray(&keys, ch->sampler->output->count, sizeof(struct NAnimVectorKey), MH_Editor);

			const struct Vec3 *values = (const struct Vec3 *)BUFFER_PTR(ch->sampler->output);
			for (cgltf_size j = 0; j < ch->sampler->output->count; ++j) {
				NAnimVectorKey vk = { { values[j].x, values[j].y, values[j].z }, time[j] };
				Rt_ArrayAdd(&keys, &vk);
			}

			nch->info.positionCount = (uint32_t)keys.count;
			nch->positionKeys = (struct NAnimVectorKey *)keys.data;
		} break;
		case cgltf_animation_path_type_rotation: {
			struct NeArray keys;
			Rt_InitArray(&keys, ch->sampler->output->count, sizeof(struct NAnimQuatKey), MH_Editor);

			const NeQuaternion *values = (const struct NeQuaternion *)BUFFER_PTR(ch->sampler->output);
			for (cgltf_size j = 0; j < ch->sampler->output->count; ++j) {
				NAnimQuatKey qk = { { values[j].x, values[j].y, values[j].z, values[j].w }, time[j] };
				Rt_ArrayAdd(&keys, &qk);
			}

			nch->info.rotationCount = (uint32_t)keys.count;
			nch->rotationKeys = (struct NAnimQuatKey *)keys.data;
		} break;
		case cgltf_animation_path_type_scale: {
			struct NeArray keys;
			Rt_InitArray(&keys, ch->sampler->output->count, sizeof(struct NAnimVectorKey), MH_Editor);

			const struct Vec3 *values = (const struct Vec3 *)BUFFER_PTR(ch->sampler->output);
			for (cgltf_size j = 0; j < ch->sampler->output->count; ++j) {
				NAnimVectorKey vk = { { values[j].x, values[j].y, values[j].z }, time[j] };
				Rt_ArrayAdd(&keys, &vk);
			}

			nch->info.scalingCount = (uint32_t)keys.count;
			nch->scalingKeys = (struct NAnimVectorKey *)keys.data;
		} break;
		}
	}

	na.channelCount = (uint32_t)channels.count;
	na.channels = (struct NAnimChannel *)channels.data;

	EdAsset_SaveNAnim(&na, path);

	struct NAnimChannel *ch;
	Rt_ArrayForEach(ch, &channels, struct NAnimChannel *) {
		Sys_Free(ch->positionKeys);
		Sys_Free(ch->rotationKeys);
		Sys_Free(ch->scalingKeys);
	}
	Rt_TermArray(&channels);
}

static int32_t
ChnCmpFunc(const struct NAnimChannel *chn, const char *name)
{
	return strncmp(chn->info.name, name, strnlen(name, sizeof(chn->info.name)));
}

/* NekoEditor
 *
 * glTF.cxx
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
