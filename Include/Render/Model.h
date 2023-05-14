#ifndef NE_RENDER_MODEL_H
#define NE_RENDER_MODEL_H

#include <Render/Types.h>
#include <Render/Material.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)
struct NeVertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
	float r, g, b, a;
};

struct NeVertexWeight
{
	int32_t j0, j1, j2, j3;
	float w0, w1, w2, w3;
};

struct NeMeshlet
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
};

struct NeMorphDelta
{
	uint32_t vertex;
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
};
#pragma pack(pop)

struct NeMesh
{
	enum NePrimitiveType type;
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	NeHandle materialResource;
	struct NeBounds bounds;
};

struct NeMorph
{
	uint64_t hash;
	char name[120];
	uint32_t deltaOffset;
	uint32_t deltaCount;
};

struct NeMorphPack
{
	uint32_t count;
	struct NeMorph *morphs;

	uint32_t deltaCount;
	struct NeMorphDelta *deltas;
};

struct NeModel
{
	struct NeMesh *meshes;
	struct NeBounds bounds;
	uint32_t vertexCount, meshCount;
	enum NeIndexType indexType;
	bool dynamic;

	struct {
		NeBufferHandle vertexBuffer, vertexWeightBuffer;
		NeBufferHandle indexBuffer;
	} gpu;

	struct {
		void *vertices;
		uint32_t vertexSize;

		void *indices;
		uint32_t indexSize;

		struct NeVertexWeight *vertexWeights;
		uint32_t vertexWeightCount;
	} cpu;

	struct {
		struct NeArray nodes, joints;
		struct NeMatrix *inverseTransforms;
		struct NeMatrix inverseTransform;
	} skeleton;

	struct {
		uint32_t count;
		struct NeMorph *info;

		uint32_t deltaCount;
		struct NeMorphDelta *deltas;
	} morph;
};

struct NeModelCreateInfo
{
	void *vertices;
	uint32_t vertexSize;

	void *indices;
	uint32_t indexSize;
	enum NeIndexType indexType;

	struct NeBone *bones;
	uint32_t boneSize;

	struct NeVertexWeight *vertexWeights;
	uint32_t vertexWeightCount;

	struct NeMesh *meshes;
	const char **materials;
	uint32_t meshCount;

	bool keepData, loadMaterials, dynamic;
};

#pragma pack(push, 1)
struct NeModelInstance
{
	struct NeMatrix mvp;
	struct NeMatrix model;
	struct NeMatrix normal;
	uint64_t vertexAddress;
	uint64_t materialAddress;
} NE_ALIGN(16);
#pragma pack(pop)

struct NeMorphPackCreateInfo
{
	uint32_t count;
	struct NeMorph *morphs;

	uint32_t deltaCount;
	struct NeMorphDelta *deltas;

	bool keepData;
};

bool Re_CreateModelResource(const char *name, const struct NeModelCreateInfo *ci, struct NeModel *mdl, NeHandle h);
bool Re_LoadModelResource(struct NeResourceLoadInfo *li, const char *args, struct NeModel *mdl, NeHandle h);
void Re_UnloadModelResource(struct NeModel *mdl, NeHandle h);

bool Re_CreateMorphPackResource(const char *name, const struct NeMorphPackCreateInfo *ci, struct NeMorphPack *mdl, NeHandle h);
bool Re_LoadMorphPackResource(struct NeResourceLoadInfo *li, const char *args, struct NeMorphPack *mdl, NeHandle h);
void Re_UnloadMorphPackResource(struct NeMorphPack *mdl, NeHandle h);

void Re_BuildMeshBounds(struct NeBounds *b, const struct NeVertex *vertices, uint32_t startVertex, uint32_t vertexCount);

#ifdef __cplusplus
}
#endif

#endif /* NE_RENDER_MODEL_H */

/* NekoEngine
 *
 * Model.h
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
