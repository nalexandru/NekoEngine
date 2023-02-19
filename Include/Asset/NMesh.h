#ifndef _NE_ASSET_NMESH_H_
#define _NE_ASSET_NMESH_H_

#include <Render/Model.h>
#include <Runtime/Array.h>

#define NMESH_4_HEADER			0x0000344853454D4Ellu	// NMESH4
#define NMESH_FOOTER			0x004853454D444E45llu	// ENDMESH
#define NMESH_SEC_FOOTER		0x0054434553444E45llu	// ENDSECT
#define NMESH_VTX_ID			0x00585456u				// VTX
#define NMESH_IDX_ID			0x00584449u				// IDX
#define NMESH_MAT_ID			0x0054414Du				// MAT
#define NMESH_BONE_ID			0x454E4F42u				// BONE
#define NMESH_WEIGHT_ID			0x54484757u				// WGHT
#define NMESH_NODE_ID			0x45444F4Eu				// NODE
#define NMESH_INVT_ID			0x54564E49u				// INVT
#define NMESH_MESH_ID			0x4853454Du				// MESH
#define NMESH_MORPH_INFO_ID		0x464E494Du				// MINF
#define NMESH_MORPH_DELTA_ID	0x544C444Du				// MINF
#define NMESH_VTXC_ID			0x43585456u				// VTXC
#define NMESH_END_ID			0x4D444E45u				// ENDM

struct NMeshSubmesh
{
	uint32_t primitiveType;
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
	char material[256];
};

struct NMeshJoint
{
	char name[256];
	float inverseBindMatrix[16];
};

struct NMeshNode
{
	char name[256];
	float transform[16];
	int32_t parentId;
	int32_t childrenIds[15];
};

struct NMeshMorphDelta
{
	uint32_t vertex;
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct NMesh
{
	uint32_t vertexCount;
	struct NeVertex *vertices;

	enum NeIndexType indexType;
	size_t indexSize;
	uint32_t indexCount;
	uint8_t *indices;

	uint32_t meshCount;
	struct NMeshSubmesh *meshes;

	uint32_t vertexWeightCount;
	struct NeVertexWeight *vertexWeights;

	uint32_t nodeCount;
	struct NMeshNode *nodes;

	uint32_t jointCount;
	struct NMeshJoint *joints;

	uint32_t morphCount;
	struct NeMorph *morphs;

	uint32_t morphDeltaCount;
	struct NeMorphDelta *morphDeltas;

	float globalInverseTransform[16];
};

#endif /* _NE_ASSET_NMESH_H_ */

/* NekoEngine
 *
 * NMesh.h
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
