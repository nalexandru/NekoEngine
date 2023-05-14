#include <Math/Math.h>
#include <System/Log.h>
#include <Render/Model.h>
#include <Animation/Animation.h>

#define SKEL_MOD	"Skeleton"

static inline void LerpVecKey(struct NeVec3 *v, float time, const struct NeArray *keys);
static inline void LerpQuatKey(struct NeQuaternion *q, float time, const struct NeArray *keys);
static void XformHierarchy(float time, const struct NeSkeleton *skel, const XMMATRIX &inverseXform, const struct NeSkeletonNode *n, const XMMATRIX &parentXform, const struct NeAnimationClip *ac, struct NeMatrix *transforms);

bool
Anim_InitSkeleton(struct NeSkeleton *s, const struct NeModel *mdl)
{
	Rt_CloneArray(&s->nodes, &mdl->skeleton.nodes, MH_Scene);
	Rt_InitPtrArray(&s->joints, mdl->skeleton.joints.count, MH_Scene);

	size_t invMatSize = sizeof(*s->inverseBind) * mdl->skeleton.joints.count;
	s->inverseBind = (struct NeMatrix *)Sys_Alloc(invMatSize, 1, MH_Scene);
	memcpy(s->inverseBind, mdl->skeleton.inverseTransforms, invMatSize);

	uint32_t *id;
	Rt_ArrayForEach(id, &mdl->skeleton.joints, uint32_t *)
		Rt_ArrayAddPtr(&s->joints, Rt_ArrayGet(&s->nodes, *id));

	struct NeSkeletonNode *node;
	Rt_ArrayForEach(node, &s->nodes, struct NeSkeletonNode *) {
		if (node->parent)
			continue;

		s->root = node;
		break;
	}

	if (!s->root) {
		Sys_LogEntry(SKEL_MOD, LOG_CRITICAL, "Root node not found");
		return false;
	}

	memcpy(&s->inverseTransform, &mdl->skeleton.inverseTransform, sizeof(s->inverseTransform));

//	Rt_ArraySort(&s->nodes, Rt_U64CmpFunc);
//	Rt_ArraySort(&s->joints, Rt_U64CmpFunc);

	return true;
}

void
Anim_UpdateSkeleton(struct NeSkeleton *s, float time, const struct NeAnimationClip *ac, struct NeMatrix *transforms)
{
	XformHierarchy(time, s, M_Load(&s->inverseTransform), s->root, XMMatrixIdentity(), ac, transforms);
}

void
Anim_TermSkeleton(struct NeSkeleton *s)
{
	Sys_Free(s->inverseBind);
	Rt_TermArray(&s->joints);
	Rt_TermArray(&s->nodes);
}

static inline void
LerpVecKey(struct NeVec3 *v, float time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		memcpy(v, &((struct NeAnimVectorKey *)Rt_ArrayGet(keys, 0))->value, sizeof(*v));
		return;
	}

	size_t idx = 0;
	for (size_t i = 0; i < keys->count - 1; ++i) {
		if (time > ((const struct NeAnimVectorKey *)Rt_ArrayGet(keys, i + 1))->time)
			continue;

		idx = i;
		break;
	}

	const struct NeAnimVectorKey *key = (struct NeAnimVectorKey *)Rt_ArrayGet(keys, idx);
	const struct NeAnimVectorKey *nextKey = (struct NeAnimVectorKey *)Rt_ArrayGet(keys, idx + 1);

	if (nextKey)
		M_Store(v, XMVectorLerp(M_Load(&key->value), M_Load(&nextKey->value),
			(float)((time - key->time) / (nextKey->time - key->time))
		));
	else
		memcpy(v, &key->value, sizeof(*v));
}

static inline void
LerpQuatKey(struct NeQuaternion *q, float time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		memcpy(q, &((struct NeAnimQuatKey *)Rt_ArrayGet(keys, 0))->value, sizeof(*q));
		return;
	}

	size_t idx = 0;
	for (size_t i = 0; i < keys->count - 1; ++i) {
		if (time > ((const struct NeAnimQuatKey *)Rt_ArrayGet(keys, i + 1))->time)
			continue;

		idx = i;
		break;
	}

	const struct NeAnimQuatKey *key = (struct NeAnimQuatKey *)Rt_ArrayGet(keys, idx);
	const struct NeAnimQuatKey *nextKey = (struct NeAnimQuatKey *)Rt_ArrayGet(keys, idx + 1);

	if (nextKey)
		M_Store(q, XMQuaternionNormalize(XMQuaternionSlerp(M_Load(&key->value), M_Load(&nextKey->value),
			(float)((time - key->time) / (nextKey->time - key->time))
		)));
	else
		memcpy(q, &key->value, sizeof(*q));
}

static void
XformHierarchy(float time, const struct NeSkeleton *skel, const XMMATRIX &inverseXform,
				const struct NeSkeletonNode *n, const XMMATRIX &parentXform,
				const struct NeAnimationClip *ac, struct NeMatrix *transforms)
{
	const struct NeAnimationChannel *ch = NULL;
	Rt_ArrayForEach(ch, &ac->channels, const struct NeAnimationChannel *) {
		if (ch->hash == n->hash)
			break;
		ch = NULL;
	}

	XMMATRIX xform = XMMatrixIdentity();

	if (ch) {
		if (ch->scalingKeys.count) {
			struct NeVec3 scale;
			LerpVecKey(&scale, time, &ch->scalingKeys);

			xform = XMMatrixMultiply(xform, XMMatrixScaling(scale.x, scale.y, scale.z));
		}

		if (ch->rotationKeys.count) {
			struct NeQuaternion rot;
			LerpQuatKey(&rot, time, &ch->rotationKeys);

			xform = XMMatrixMultiply(xform, XMMatrixRotationQuaternion(XMLoadFloat4A((XMFLOAT4A *)&rot)));
		}

		if (ch->positionKeys.count) {
			struct NeVec3 pos;
			LerpVecKey(&pos, time, &ch->positionKeys);

			xform = XMMatrixMultiply(xform, XMMatrixTranslation(pos.x, pos.y, pos.z));
		}
	} else {
		xform = M_Load(&n->xform);
	}

	xform = XMMatrixMultiply(xform, parentXform);

	const struct NeSkeletonNode *b = NULL;
	Rt_ArrayForEachPtr(b, &skel->joints, const struct NeSkeletonNode *) {
		if (b->hash != n->hash)
			continue;

		M_Store(&transforms[miwa_rtafei], XMMatrixMultiply(XMMatrixMultiply(M_Load(&skel->inverseBind[miwa_rtafei]), xform), inverseXform));
		break;
	}

	struct NeSkeletonNode *child;
	Rt_ArrayForEachPtr(child, &n->children, struct NeSkeletonNode *)
		XformHierarchy(time, skel, inverseXform, child, xform, ac, transforms);
}

/* NekoEngine
 *
 * Skeleton.cxx
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
