#include <string.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <stb_image.h>
#include <stb_image_write.h>

#include <Math/Math.h>
#include <Asset/NAnim.h>
#include <Asset/NMesh.h>
#include <System/Log.h>
#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>
#include <Runtime/Runtime.h>

#define ASIMPMOD	"AssimpImport"

using namespace Assimp;

ED_ASSET_IMPORTER(Assimp);

static void _ConvertMesh(struct aiMesh *mesh, struct NMesh *nm);

void _BuildNodes(struct aiNode *node, int32_t parentId, struct NeArray *nodes);

static inline void _SaveMaterial(const struct aiMaterial *mat, const char *name, const char *path);
static inline void _SaveAnimation(const struct aiAnimation *anim, const char *path);

static bool
_MatchAsset(const char *path)
{
	return strstr(path, ".fbx") || strstr(path, ".obj") || strstr(path, ".dae");
}

static bool
_ImportAsset(const char *path)
{
	bool rc = false;
	struct NMesh nm = { 0 };
	size_t vertexCount = 0, indexCount = 0, jointCount = 0;

	EdGUI_ShowProgressDialog("Importing asset...");

	char *baseDir = strdup(path);
	char *name = strrchr(baseDir, ED_DIR_SEPARATOR);
	*name++ = 0x0;

	char *ptr = strrchr(name, '.');
	if (ptr)
		*ptr = 0x0;
	
	Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

	const struct aiScene *scn = importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (!scn || scn->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scn->mRootNode) {
		EdGUI_MessageBox("Error", "Failed to read asset file", MB_Error);
		goto exit;
	}
	
	if (scn->HasAnimations()) {
		struct NeMatrix git;
		M_Store(&git, XMMatrixInverse(NULL, XMLoadFloat4x4((XMFLOAT4X4 *)&scn->mRootNode->mTransformation)));
		memcpy(nm.globalInverseTransform, &git, sizeof(nm.globalInverseTransform));
	}
	
	EdGUI_UpdateProgressDialog("Converting meshes...");
	
	for (uint32_t i = 0; i < scn->mNumMeshes; ++i) {
		vertexCount += scn->mMeshes[i]->mNumVertices;
		indexCount += scn->mMeshes[i]->mNumFaces * 3;
		jointCount += scn->mMeshes[i]->mNumBones;
	}

	if (jointCount)
		nm.vertexWeightCount = (uint32_t)vertexCount;

	nm.indexType = indexCount >= UINT16_MAX ? IT_UINT_32 : IT_UINT_16;

	switch (nm.indexType) {
	case IT_UINT_16: nm.indexSize = sizeof(uint16_t); break;
	case IT_UINT_32: nm.indexSize = sizeof(uint32_t); break;
	}
	
	nm.vertices = (NeVertex *)calloc(vertexCount, sizeof(*nm.vertices));
	nm.indices = (uint8_t *)calloc(indexCount, nm.indexSize);
	nm.meshes = (NMeshSubmesh *)calloc(scn->mNumMeshes, sizeof(*nm.meshes));
	nm.joints = (NMeshJoint *)calloc(jointCount, sizeof(*nm.joints));

	if (nm.vertexWeightCount)
		nm.vertexWeights = (NeVertexWeight *)calloc(nm.vertexWeightCount, sizeof(*nm.vertexWeights));

	struct NeArray nodes;
	Rt_InitArray(&nodes, 10, sizeof(struct NMeshNode), MH_Editor);
	_BuildNodes(scn->mRootNode, -1, &nodes);

	nm.nodes = (struct NMeshNode *)nodes.data;
	nm.nodeCount = (uint32_t)nodes.count;

	for (uint32_t i = 0; i < scn->mNumMeshes; ++i)
		_ConvertMesh(scn->mMeshes[i], &nm);
	
	char pathBuff[4096];
	
	EdGUI_UpdateProgressDialog("Converting materials...");
	if (scn->mNumMaterials) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cMaterials%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}
	
	for (uint32_t i = 0; i < scn->mNumMaterials; ++i) {
		struct aiString matName;
		aiGetMaterialString(scn->mMaterials[i], AI_MATKEY_NAME, &matName);

		// assign material to mesh
		for (uint32_t j = 0; j < nm.meshCount; ++j) {
			struct NMeshSubmesh *sm = &nm.meshes[j];
			if (!Rt_IsNumeric(sm->material))
				continue;

			int id = atoi(sm->material);
			if (id == i)
				snprintf(sm->material, sizeof(sm->material), "/Materials/%s/%s.mat", name, matName.data);
		}

		snprintf(pathBuff, sizeof(pathBuff), "%s%cMaterials%c%s%c%s.mat", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
				 name, ED_DIR_SEPARATOR, matName.data);
		_SaveMaterial(scn->mMaterials[i], name, pathBuff);
	}

	for (uint32_t i = 0; i < nm.meshCount; ++i) {
		struct NMeshSubmesh *sm = &nm.meshes[i];
		if (!Rt_IsNumeric(sm->material))
			continue;

		snprintf(sm->material, sizeof(sm->material), "%s", "/Materials/Default.mat");
	}

	//Asset_OptimizeNMesh(&nm);

	snprintf(pathBuff, sizeof(pathBuff), "%s%cModels%c%s.nmesh", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
	Asset_SaveNMesh(&nm, pathBuff);

	EdGUI_UpdateProgressDialog("Converting animations...");
	if (scn->mNumAnimations) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cAnimations%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}
	
	for (uint32_t i = 0; i < scn->mNumAnimations; ++i) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cAnimations%c%s%c%s.nanim", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, scn->mAnimations[i]->mName.data);
		_SaveAnimation(scn->mAnimations[i], pathBuff);
	}
	
	EdGUI_UpdateProgressDialog("Converting textures...");
	if (scn->mNumTextures) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
		if (!Sys_DirectoryExists(pathBuff))
			Sys_CreateDirectory(pathBuff);
	}
	
	for (uint32_t i = 0; i < scn->mNumTextures; ++i) {
		struct aiTexture *tex = scn->mTextures[i];
		int w = tex->mWidth, h = tex->mHeight, c = 4;
		stbi_uc *imageData = NULL;
		
		if (tex->mHeight) {
			imageData = stbi_load_from_memory((stbi_uc *)tex->pcData, tex->mWidth, &w, &h, &c, 4);
		} else {
			imageData = (stbi_uc *)calloc(tex->mWidth * tex->mHeight * 4, 1);
			uint32_t *src = (uint32_t *)tex->pcData;
			uint32_t *dst = (uint32_t *)imageData;
			
			for (uint32_t i = 0; i < tex->mWidth * tex->mHeight; ++i)
				dst[i] = ((src[i] & 0x00FF0000) << 8) | ((src[i] & 0x0000FF00) << 8) |
							((src[i] & 0x000000FF) << 8) | ((src[i] & 0xFF000000) >> 24);
		}

		snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s%c%s.png", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
																			name, ED_DIR_SEPARATOR, tex->mFilename.data);
		stbi_write_png(pathBuff, w, h, 4, imageData, 0);
		
		free(imageData);
	}

	rc = true;
exit:

	EdGUI_HideProgressDialog();
	delete scn;
	
	free(baseDir);
	
	return rc;
}

void
_ConvertMesh(struct aiMesh *mesh, struct NMesh *nm)
{
	struct NMeshSubmesh *dstMesh = &nm->meshes[nm->meshCount++];
	dstMesh->primitiveType = PT_TRIANGLES;
	
	dstMesh->vertexOffset = nm->vertexCount;
	dstMesh->vertexCount = mesh->mNumVertices;
	dstMesh->indexOffset = nm->indexCount;
	dstMesh->indexCount = mesh->mNumFaces * 3;
	
	snprintf(dstMesh->material, sizeof(dstMesh->material), "%d", mesh->mMaterialIndex);
	
	if (nm->indexType == IT_UINT_16) {
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			((uint16_t *)nm->indices)[nm->indexCount++] = (uint16_t)mesh->mFaces[i].mIndices[0];
			((uint16_t *)nm->indices)[nm->indexCount++] = (uint16_t)mesh->mFaces[i].mIndices[1];
			((uint16_t *)nm->indices)[nm->indexCount++] = (uint16_t)mesh->mFaces[i].mIndices[2];
		}
	} else {
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			((uint32_t *)nm->indices)[nm->indexCount++] = mesh->mFaces[i].mIndices[0];
			((uint32_t *)nm->indices)[nm->indexCount++] = mesh->mFaces[i].mIndices[1];
			((uint32_t *)nm->indices)[nm->indexCount++] = mesh->mFaces[i].mIndices[2];
		}
	}
	
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
		struct NeVertex *v = &nm->vertices[nm->vertexCount++];

		v->x = mesh->mVertices[i].x;
		v->y = mesh->mVertices[i].y;
		v->z = mesh->mVertices[i].z;
		
		if (mesh->mNormals) {
			v->nx = mesh->mNormals[i].x;
			v->ny = mesh->mNormals[i].y;
			v->nz = mesh->mNormals[i].z;
		}

		if (mesh->mTangents) {
			v->tx = mesh->mTangents[i].x;
			v->ty = mesh->mTangents[i].y;
			v->tz = mesh->mTangents[i].z;
		}
		
		if (mesh->mTextureCoords[0]) {
			v->u = mesh->mTextureCoords[0][i].x;
			v->v = mesh->mTextureCoords[0][i].y;
		}
		
		if (mesh->mColors[0]) {
			v->r = mesh->mColors[0][i].r;
			v->g = mesh->mColors[0][i].g;
			v->b = mesh->mColors[0][i].b;
			v->a = mesh->mColors[0][i].a;
		} else {
			v->r = 1.f; v->g = 1.f; v->b = 1.f; v->a = 1.f;
		}
	}
	
	for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
		struct NMeshJoint *j = &nm->joints[nm->jointCount++];

		snprintf(j->name, sizeof(j->name), "%s", mesh->mBones[i]->mName.data);
		memcpy(j->inverseBindMatrix, &mesh->mBones[i]->mOffsetMatrix, sizeof(j->inverseBindMatrix));
		
		for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
			struct NeVertexWeight *vw = &nm->vertexWeights[mesh->mBones[i]->mWeights[j].mVertexId + dstMesh->vertexOffset];
			
			*(&vw->j0 + j) = i;
			*(&vw->w0 + j) = mesh->mBones[i]->mWeights[j].mWeight;
		}
	}
}

void
_BuildNodes(struct aiNode *node, int32_t parentId, struct NeArray *nodes)
{
	int32_t id = (int32_t)nodes->count;
	struct NMeshNode *n = (struct NMeshNode *)Rt_ArrayAllocate(nodes);
	
	snprintf(n->name, sizeof(n->name), "%s", node->mName.data);
	memcpy(n->transform, &node->mTransformation, sizeof(n->transform));
	n->parentId = parentId;
	
	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		_BuildNodes(node->mChildren[i], id, nodes);
}

void
_SaveMaterial(const aiMaterial *mat, const char *name, const char *path)
{
	float f;
	aiColor4D color;
	aiString str;

	FILE *fp = fopen(path, "w");

	fprintf(fp, "{\n\t\"id\": \"NeMaterial\",\n\t\"version\": 1,\n\t\"name\": \"%s\",\n\t\"type\": \"Default\"", name);

#define ARG_F(n, v) fprintf(fp, ",\n\t\"%s\": %.02f", n, v)
#define ARG_C(n, v) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f, %.02f\"", n, v.r, v.g, v.b, v.a)
#define ARG_IMG(n, v) fprintf(fp, ",\n\t\"%s\": \"/Textures/%s/%s\"", n, name, v)
	
	if (mat->Get(AI_MATKEY_TRANSPARENCYFACTOR, f) == AI_SUCCESS)
		ARG_F("AlphaCutoff", f);

	if (mat->GetTexture(aiTextureType_NORMALS, 0, &str) == AI_SUCCESS)
		ARG_IMG("NormalMap", str.data);

	if (mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str) == AI_SUCCESS)
		ARG_IMG("OcclusionMap", str.data);

	if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
		ARG_C("DiffuseColor", color);

	if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS)
		ARG_IMG("DiffuseMap", str.data);

	if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
		ARG_C("EmissiveColor", color);

	if (mat->GetTexture(aiTextureType_EMISSIVE, 0, &str) == AI_SUCCESS)
		ARG_IMG("EmissiveMap", str.data);

	if (mat->Get(AI_MATKEY_METALLIC_FACTOR, f) == AI_SUCCESS)
		ARG_F("Metallic", f);

	if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, f) == AI_SUCCESS)
		ARG_F("Roughness", f);

	/* TODO: Combine these two
	if (mat->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("MetallicRoughnessMap", str.data);

	if (mat->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("MetallicRoughnessMap", str.data);
	*/

	if (mat->Get(AI_MATKEY_CLEARCOAT_FACTOR, f) == AI_SUCCESS)
		ARG_F("ClearCoat", f);

	if (mat->GetTexture(AI_MATKEY_CLEARCOAT_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("ClearCoatMap", str.data);

	if (mat->GetTexture(AI_MATKEY_CLEARCOAT_NORMAL_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("ClearCoatNormalMap", str.data);

	if (mat->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, f) == AI_SUCCESS)
		ARG_F("ClearCoatRoughness", f);

	if (mat->GetTexture(AI_MATKEY_CLEARCOAT_ROUGHNESS_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("ClearCoatRoughnessMap", str.data);

	if (mat->Get(AI_MATKEY_TRANSMISSION_FACTOR, f) == AI_SUCCESS)
		ARG_F("Transmission", f);

	if (mat->GetTexture(AI_MATKEY_TRANSMISSION_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("TransmissionMap", str.data);

	if (mat->Get(AI_MATKEY_SPECULAR_FACTOR, f) == AI_SUCCESS)
		ARG_F("SpecularWeight", f);

	if (mat->Get(AI_MATKEY_SHEEN_COLOR_FACTOR, f) == AI_SUCCESS)
		ARG_F("Sheen", f);

	if (mat->GetTexture(AI_MATKEY_SHEEN_COLOR_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("SheenMap", str.data);

	if (mat->Get(AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, f) == AI_SUCCESS)
		ARG_F("SheenRoughness", f);

	if (mat->GetTexture(AI_MATKEY_SHEEN_ROUGHNESS_TEXTURE, &str) == AI_SUCCESS)
		ARG_IMG("SheenRoughnessMap", str.data);

	fprintf(fp, "\n}\n");
	fclose(fp);
}

void
_SaveAnimation(const struct aiAnimation *anim, const char *path)
{
	struct NAnim nanim = { .channelCount = anim->mNumChannels };
	
	nanim.info.duration = anim->mDuration / anim->mTicksPerSecond;
	nanim.info.ticks = anim->mTicksPerSecond;
	snprintf(nanim.info.name, sizeof(nanim.info.name), "%s", anim->mName.data);
	
	nanim.channels = (NAnimChannel *)Sys_Alloc(sizeof(*nanim.channels), nanim.channelCount, MH_Editor);
	for (uint32_t i = 0; i < nanim.channelCount; ++i) {
		const struct aiNodeAnim *ch = anim->mChannels[i];
		struct NAnimChannel *nch = &nanim.channels[i];

		snprintf(nch->name, sizeof(nch->name), "%s", ch->mNodeName.data);

		nch->positionCount = ch->mNumPositionKeys;
		nch->rotationCount = ch->mNumRotationKeys;
		nch->scalingCount = ch->mNumScalingKeys;

		nch->positionKeys = (NAnimVectorKey *)Sys_Alloc(sizeof(*nch->positionKeys), nch->positionCount, MH_Editor);
		nch->rotationKeys = (NAnimQuatKey *)Sys_Alloc(sizeof(*nch->rotationKeys), nch->rotationCount, MH_Editor);
		nch->scalingKeys = (NAnimVectorKey *)Sys_Alloc(sizeof(*nch->scalingKeys), nch->scalingCount, MH_Editor);

		for (uint32_t j = 0; j < nch->positionCount; ++j) {
			const struct aiVectorKey *vk = &ch->mPositionKeys[j];
			struct NAnimVectorKey *nvk = &nch->positionKeys[j];

			nvk->time = vk->mTime;
			nvk->val[0] = vk->mValue.x;
			nvk->val[1] = vk->mValue.y;
			nvk->val[2] = vk->mValue.z;
		}

		for (uint32_t j = 0; j < nch->rotationCount; ++j) {
			const struct aiQuatKey *qk = &ch->mRotationKeys[j];
			struct NAnimQuatKey *nqk = &nch->rotationKeys[j];

			nqk->time = qk->mTime;
			nqk->quat[0] = qk->mValue.x;
			nqk->quat[1] = qk->mValue.y;
			nqk->quat[2] = qk->mValue.z;
			nqk->quat[3] = qk->mValue.w;
		}

		for (uint32_t j = 0; j < nch->scalingCount; ++j) {
			const struct aiVectorKey *vk = &ch->mScalingKeys[j];
			struct NAnimVectorKey *nvk = &nch->scalingKeys[j];

			nvk->time = vk->mTime;
			nvk->val[0] = vk->mValue.x;
			nvk->val[1] = vk->mValue.y;
			nvk->val[2] = vk->mValue.z;
		}
	}

	Asset_SaveNAnim(&nanim, path);

	for (uint32_t i = 0; i < nanim.channelCount; ++i) {
		struct NAnimChannel *ch = &nanim.channels[i];
		Sys_Free(ch->positionKeys);
		Sys_Free(ch->rotationKeys);
		Sys_Free(ch->scalingKeys);
	}

	Sys_Free(nanim.channels);
}

/* NekoEditor
 *
 * Assimp.cxx
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
