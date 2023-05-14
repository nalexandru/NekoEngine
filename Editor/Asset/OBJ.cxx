#include <stdlib.h>
#include <string.h>

#include <vector>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

using namespace std;
using namespace tinyobj;

#define OBJ_MOD		"OBJImport"

struct TextureInfo
{
	uint64_t hash;
	char src[4096];
	char dst[4096];
};

ED_ASSET_IMPORTER(OBJ);

static inline void AddTexture(FILE *fp, const char *name, const char *dir, const char *baseDir, struct NeArray *textures);
static inline void SaveMaterial(const material_t &mat, const char *name, const char *path, const char *baseDir, struct NeArray *textures);
static inline uint8_t *BuildMetallicRoughness(const char *m, const char *r, uint16_t *w, uint16_t *h);

static bool
OBJ_MatchAsset(const char *path)
{
	return strstr(path, ".obj");
}

static bool
OBJ_ImportAsset(const char *path, const struct NeAssetImportOptions *options)
{
	bool rc = false;
	struct NMesh nm = { 0 };

	EdGUI_ShowProgressDialog("Importing asset...");

	attrib_t attrib{};
	set<uint32_t> vtxIds{};
	vector<shape_t> shapes{};
	vector<material_t> materials{};
	string warn{}, err{};
	char pathBuff[4096]{};

	char *baseDir = strdup(path);
	char *name = strrchr(baseDir, ED_DIR_SEPARATOR);
	*name++ = 0x0;

	char *ptr = strrchr(name, '.');
	if (ptr)
		*ptr = 0x0;

	if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, path, baseDir)) {
		EdGUI_MessageBox("Error", "Failed to read asset file", MB_Error);
		goto exit;
	}

	snprintf(pathBuff, sizeof(pathBuff), "%s%cModels%c%s.nmesh", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);

	EdGUI_UpdateProgressDialog("Converting meshes...");

	nm.indexType = IT_UINT_32;
	nm.indexSize = sizeof(uint32_t);
	struct NeArray vertices, indices, meshes;
	Rt_InitArray(&vertices, attrib.vertices.size() / 3, sizeof(*nm.vertices), MH_Editor);
	Rt_InitArray(&indices, attrib.vertices.size() / 3, sizeof(uint32_t), MH_Editor);
	Rt_InitArray(&meshes, shapes.size(), sizeof(*nm.meshes), MH_Editor);

	Rt_FillArray(&vertices);

	for (const shape_t &shape : shapes) {
		struct NMeshSubmesh *sm = (struct NMeshSubmesh *)Rt_ArrayAllocate(&meshes);

		sm->vertexOffset = 0;
		sm->indexOffset = (uint32_t)indices.count;

		vtxIds.clear();
		for (const index_t &idx : shape.mesh.indices) {
			vtxIds.emplace(idx.vertex_index);
			struct NeVertex *vtx = (struct NeVertex *)Rt_ArrayGet(&vertices, idx.vertex_index);

			vtx->x = attrib.vertices[3 * idx.vertex_index + 0];
			vtx->y = attrib.vertices[3 * idx.vertex_index + 1];
			vtx->z = attrib.vertices[3 * idx.vertex_index + 2];

			vtx->r = vtx->g = vtx->b = vtx->a = 1.f;

			if (idx.normal_index >= 0) {
				vtx->nx = attrib.normals[3 * idx.normal_index + 0];
				vtx->ny = attrib.normals[3 * idx.normal_index + 1];
				vtx->nz = attrib.normals[3 * idx.normal_index + 2];
			}

			if (idx.texcoord_index >= 0) {
				vtx->u = attrib.texcoords[2 * idx.texcoord_index + 0];
				vtx->v = attrib.texcoords[2 * idx.texcoord_index + 1];
			}

			uint32_t index = idx.vertex_index;
			Rt_ArrayAdd(&indices, &index);
		}

		if (shape.mesh.material_ids[0] != -1) {
			const material_t &mat = materials[shape.mesh.material_ids[0]];
			snprintf(sm->material, sizeof(sm->material), "/Materials/%s/%s.mat", name, mat.name.c_str());
		} else {
			strlcpy(sm->material, "/Materials/Default.mat", sizeof(sm->material));
		}

		sm->vertexCount = (uint32_t)vtxIds.size();
		sm->indexCount = (uint32_t)indices.count - sm->indexOffset;
	}

	nm.vertices = (struct NeVertex *)vertices.data;
	nm.vertexCount = (uint32_t)vertices.count;
	nm.indices = indices.data;
	nm.indexCount = (uint32_t)indices.count;
	nm.meshes = (struct NMeshSubmesh *)meshes.data;
	nm.meshCount = (uint32_t)meshes.count;

	EdAsset_OptimizeNMesh(&nm);
	if (options->inMemory)
		memcpy(options->dst, &nm, sizeof(nm));
	else
		EdAsset_SaveNMesh(&nm, pathBuff);

	if (options->onlyGeometry) {
		rc = true;
		goto exit;
	}

	struct NeArray textures;
	Rt_InitArray(&textures, 10, sizeof(struct TextureInfo), MH_Editor);

	EdGUI_UpdateProgressDialog("Converting materials...");

	snprintf(pathBuff, sizeof(pathBuff), "%s%cMaterials%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
	if (!Sys_DirectoryExists(pathBuff))
		Sys_CreateDirectory(pathBuff);

	for (const material_t &mat : materials) {
		snprintf(pathBuff, sizeof(pathBuff), "%s%cMaterials%c%s%c%s.mat", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR,
				 name, ED_DIR_SEPARATOR, mat.name.c_str());
		SaveMaterial(mat, name, pathBuff, baseDir, &textures);
	}

	EdGUI_UpdateProgressDialog("Converting textures...");

	snprintf(pathBuff, sizeof(pathBuff), "%s%cTextures%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, name);
	if (!Sys_DirectoryExists(pathBuff))
		Sys_CreateDirectory(pathBuff);

	const struct TextureInfo *ti;
	Rt_ArrayForEach(ti, &textures, const struct TextureInfo *) {
		FILE *fp = fopen(ti->src, "rb");
		if (!fp)
			continue;

		fseek(fp, 0, SEEK_END);
		const size_t sz = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		uint8_t *data = (uint8_t *)Sys_Alloc(sz, 1, MH_Asset);
		if (fread(data, 1, sz, fp) == sz)
			EdAsset_ConvertToPNG(data, sz, ti->dst);

		Sys_Free(data);
		fclose(fp);
	}

	rc = true;
exit:
	EdGUI_HideProgressDialog();

	if (!options->inMemory)
		EdAsset_FreeNMesh(&nm);

	free(baseDir);

	return rc;
}

static inline void
AddTexture(FILE *fp, const char *id, const char *name, const char *dir, const char *baseDir, struct NeArray *textures)
{
	char buff[256];
	strlcpy(buff, name, sizeof(buff));
	char *eptr = strrchr(buff, '.');
	if (eptr) {
		if (eptr - buff >= 5)
			eptr[1] = 'p'; eptr[2] = 'n'; eptr[3] = 'g'; eptr[4] = 0x0;
	} else if (strlen(buff) <= sizeof(buff) - 5) {
		strlcat(buff, ".png", sizeof(buff));
	}
	fprintf(fp, ",\n\t\"%s\": \"/Textures/%s/%s\"", id, dir, buff);

	struct TextureInfo ti{};

	snprintf(ti.src, sizeof(ti.src), "%s%c%s", baseDir, ED_DIR_SEPARATOR, name);
	ti.hash = Rt_HashString(ti.src);

	if (Rt_ArrayFind(textures, &ti.hash, Rt_U64CmpFunc))
		return;

	snprintf(ti.dst, sizeof(ti.dst), "%s%cTextures%c%s%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, dir, ED_DIR_SEPARATOR, buff);
	Rt_ArrayAdd(textures, &ti);
}

void
SaveMaterial(const material_t &mat, const char *name, const char *path, const char *baseDir, struct NeArray *textures)
{
	FILE *fp = fopen(path, "w");
	fprintf(fp, "{\n\t\"id\": \"NeMaterial\",\n\t\"version\": 1,\n\t\"name\": \"%s\",\n\t\"type\": \"Default\"", mat.name.c_str());

#define ARG_F(n, v) fprintf(fp, ",\n\t\"%s\": %.02f", (n), (v))
#define ARG_C(n, v, a) fprintf(fp, ",\n\t\"%s\": \"%.02f, %.02f, %.02f, %.02f\"", (n), (v)[0], (v)[1], (v)[2], (a))

	ARG_C("DiffuseColor", mat.diffuse, 1.f);
	if (mat.diffuse_texname.length())
		AddTexture(fp, "DiffuseMap", mat.diffuse_texname.c_str(), name, baseDir, textures);

	ARG_C("EmissiveColor", mat.emission, 1.f);
	if (mat.emissive_texname.length())
		AddTexture(fp, "EmissiveMap", mat.emissive_texname.c_str(), name, baseDir, textures);

	if (mat.normal_texname.length())
		AddTexture(fp, "NormalMap", mat.normal_texname.c_str(), name, baseDir, textures);

	ARG_F("Metallic", mat.metallic);
	ARG_F("Roughness", mat.roughness);

	/*uint16_t w, h;
	uint8_t *metallicRoughnessData = BuildMetallicRoughness(metallic, roughness, &w, &h);
	stbi_write_png("", w, h, 4, metallicRoughnessData, 0);
	ARG_IMG("MetallicRoughnessMap", "");
	free(metallicRoughnessData);*/

	//ARG_F("Transmission", mat.transmittance);
	ARG_F("ClearCoat", mat.clearcoat_thickness);
	ARG_F("ClearCoatRoughness", mat.clearcoat_roughness);
	ARG_F("Sheen", mat.sheen);
	ARG_F("SpecularWeight", mat.shininess);

	if (mat.sheen_texname.length())
		AddTexture(fp, "SheenMap", mat.sheen_texname.c_str(), name, baseDir, textures);

	ARG_F("IOR", mat.ior);

#undef ARG_F
#undef ARG_C
#undef ARG_IMG

	fprintf(fp, "\n}\n");
	fclose(fp);
}

static inline uint8_t *
BuildMetallicRoughness(const char *m, const char *r, uint16_t *w, uint16_t *h)
{
	int mWidth, mHeight;
	stbi_uc *metallicData = NULL;//stbi_load(m, &mWidth, &mHeight, &c, 1);
	assert(metallicData);

	int rWidth, rHeight;
	stbi_uc *roughnessData = NULL;//stbi_load(r, &rWidth, &rHeight, &c, 1);
	assert(metallicData);

	if (mWidth != rWidth || mHeight != rHeight) {
		// resize images
	}

	uint8_t *dst = (uint8_t *)calloc(mWidth * mHeight * 4, sizeof(*dst));
	for (size_t i = 0; i < mWidth * mHeight * 2; i) {
		dst[i] = metallicData[i];
		dst[i + 1] = roughnessData[i];

		i += 4;
	}

	*w = mWidth;
	*h = mHeight;

	return dst;
}

/* NekoEditor
 *
 * OBJ.cxx
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
