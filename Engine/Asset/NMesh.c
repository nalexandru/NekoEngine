#include <Asset/NMesh.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Runtime/Array.h>
#include <System/Log.h>
#include <Animation/Skeleton.h>

#define NMESH_MOD	"NMesh"

bool
E_LoadNMeshAsset(struct NeStream *stm, struct NeModel *m)
{
	ASSET_INFO;

	char *matName = Sys_Alloc(sizeof(*matName), 258, MH_Asset);

	ASSET_CHECK_GUARD(NMESH_4_HEADER);

	while (!E_EndOfStream(stm)) {
		ASSET_READ_ID();

		if (a.id == NMESH_VTX_ID) {
			m->cpu.vertexSize = a.size;
			m->cpu.vertices = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, m->cpu.vertices, a.size);
		} else if (a.id == NMESH_VTXC_ID) {
			if (a.size != sizeof(m->vertexCount))
				goto error;
			E_ReadStream(stm, &m->vertexCount, sizeof(m->vertexCount));
		} else if (a.id == NMESH_IDX_ID) {
			E_ReadStream(stm, &m->indexType, sizeof(m->indexType));
			m->cpu.indexSize = a.size;
			m->cpu.indices = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, m->cpu.indices, a.size);
		} else if (a.id == NMESH_BONE_ID) {
			size_t count = a.size / sizeof(struct NMeshJoint);

			struct NMeshJoint *meshJoints = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, meshJoints, a.size);

			Rt_InitArray(&m->skeleton.bones, count, sizeof(struct NeBone), MH_Asset);
			Rt_FillArray(&m->skeleton.bones);
			for (size_t i = 0; i < count; ++i) {
				const struct NMeshJoint *mj = &meshJoints[i];
				struct NeBone *b = Rt_ArrayGet(&m->skeleton.bones, i);

				b->hash = Rt_HashString(mj->name);
				memcpy(b->name, mj->name, sizeof(b->name));
				memcpy(&b->offset, mj->inverseBindMatrix, sizeof(b->offset));
			}

			Sys_Free(meshJoints);
		} else if (a.id == NMESH_WEIGHT_ID) {
			m->cpu.vertexWeightSize = a.size;
			m->cpu.vertexWeights = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, m->cpu.vertexWeights, a.size);
		} else if (a.id == NMESH_NODE_ID) {
			size_t count = a.size / sizeof(struct NMeshNode);

			struct NMeshNode *meshNodes = Sys_Alloc(a.size, 1, MH_Asset);
			E_ReadStream(stm, meshNodes, a.size);

			Rt_InitArray(&m->skeleton.nodes, count, sizeof(struct NeSkeletonNode), MH_Asset);
			Rt_FillArray(&m->skeleton.nodes);
			for (size_t i = 0; i < count; ++i) {
				const struct NMeshNode *mn = &meshNodes[i];
				struct NeSkeletonNode *n = Rt_ArrayGet(&m->skeleton.nodes, i);

				n->hash = Rt_HashString(mn->name);
				memcpy(n->name, mn->name, sizeof(n->name));
				memcpy(&n->xform, mn->transform, sizeof(n->xform));

				if (mn->parentId < 0)
					continue;

				struct NeSkeletonNode *p = Rt_ArrayGet(&m->skeleton.nodes, mn->parentId);
				n->parent = p;

				if (!p->children.data)
					Rt_InitPtrArray(&p->children, 2, MH_Asset);

				Rt_ArrayAddPtr(&p->children, n);
			}

			Sys_Free(meshNodes);
		} else if (a.id == NMESH_INVT_ID) {
			if (a.size != sizeof(m->skeleton.globalInverseTransform.m))
				goto error;

			E_ReadStream(stm, m->skeleton.globalInverseTransform.m, sizeof(m->skeleton.globalInverseTransform.m));
		} else if (a.id == NMESH_MESH_ID) {
			struct NMeshSubmesh submesh = {0 };

			m->meshCount = a.size;
			m->meshes = Sys_Alloc(sizeof(*m->meshes), m->meshCount, MH_Asset);

			for (uint32_t i = 0; i < a.size; ++i) {
				if (E_ReadStream(stm, &submesh, sizeof(submesh)) != sizeof(submesh))
					goto error;

				memset(matName, 0x0, 258);
				snprintf(matName, 258, "%s:%u", submesh.material, submesh.primitiveType);

				m->meshes[i].type = submesh.primitiveType;
				m->meshes[i].vertexOffset = submesh.vertexOffset;
				m->meshes[i].vertexCount = submesh.vertexCount;
				m->meshes[i].indexOffset = submesh.indexOffset;
				m->meshes[i].indexCount = submesh.indexCount;
				m->meshes[i].materialResource = E_LoadResource(matName, RES_MATERIAL);

				Re_BuildMeshBounds(&m->meshes[i].bounds, m->cpu.vertices, submesh.vertexOffset, submesh.vertexCount);
			}
		} else if (a.id == NMESH_MORPH_INFO_ID) {
			m->morph.count = a.size / sizeof(struct NeMorph);
			m->morph.info = Sys_Alloc(a.size, 1, MH_Asset);
			if (!E_ReadStream(stm, m->morph.info, a.size))
				goto error;
		} else if (a.id == NMESH_MORPH_DELTA_ID) {
			m->morph.deltaCount = a.size / sizeof(struct NeMorphDelta);
			m->morph.deltas = Sys_Alloc(a.size, 1, MH_Asset);
			if (E_ReadStream(stm, m->morph.deltas, a.size) != a.size)
				goto error;
		} else if (a.id == NMESH_END_ID) {
			E_StreamSeek(stm, -((int64_t)sizeof(a)), IO_SEEK_CUR);
			break;
		} else {
			Sys_LogEntry(NMESH_MOD, LOG_WARNING, "Unknown section id = 0x%x, size = %d", a.id, a.size);
			E_StreamSeek(stm, a.size, IO_SEEK_CUR);
		}

		ASSET_CHECK_GUARD(NMESH_SEC_FOOTER);
	}

	ASSET_CHECK_GUARD(NMESH_FOOTER);

	Sys_Free(matName);

	return true;

error:
	Rt_TermArray(&m->skeleton.bones);
	Rt_TermArray(&m->skeleton.nodes);

	Sys_Free(m->cpu.vertices);
	Sys_Free(m->cpu.indices);
	Sys_Free(m->meshes);
	Sys_Free(matName);

	return false;
}

/* NekoEngine
 *
 * NMesh.c
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
