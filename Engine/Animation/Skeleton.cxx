#include <Math/Math.h>
#include <Engine/Engine.h>
#include <Animation/Animation.h>

static inline void _LerpVecKey(struct NeVec3 *v, double time, const struct NeArray *keys);
static inline void _LerpQuatKey(struct NeQuaternion *q, double time, const struct NeArray *keys);
static void _XformHierarchy(double time, const struct NeSkeleton *skel, const struct NeSkeletonNode *n, const XMMATRIX &parentXform, const struct NeAnimationClip *ac, struct NeMatrix *transforms);

bool
Anim_InitSkeleton(struct NeSkeleton *s, const struct NeArray *bones, const struct NeArray *nodes, const struct NeMatrix *git)
{
	Rt_CloneArray(&s->bones, bones, MH_Scene);
	Rt_CloneArray(&s->nodes, nodes, MH_Scene);

	//Rt_ArraySort(&s->nodes, Rt_U64CmpFunc);
	Rt_ArraySort(&s->bones, Rt_U64CmpFunc);

	s->root = (struct NeSkeletonNode *)Rt_ArrayGet(&s->nodes, 0);
	memcpy(&s->globalInverseTransform, git, sizeof(s->globalInverseTransform));

	return false;
}

void
Anim_UpdateSkeleton(struct NeSkeleton *s, double time, const struct NeAnimationClip *ac, struct NeMatrix *transforms)
{
	const double tTime = time * (ac->ticks != 0 ? ac->ticks : 25.0);
	const double aTime = M_Mod(tTime, ac->duration);

	XMMATRIX xform = XMMatrixIdentity();
	_XformHierarchy(aTime, s, (struct NeSkeletonNode *)Rt_ArrayGet(&s->nodes, 0), xform, ac, transforms);
}

void
Anim_TermSkeleton(struct NeSkeleton *s)
{
	Rt_TermArray(&s->bones);
	Rt_TermArray(&s->nodes);
}

static inline void
_LerpVecKey(struct NeVec3 *v, double time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		memcpy(v, &((struct NeAnimVectorKey *)Rt_ArrayGet(keys, 0))->val, sizeof(*v));
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

	XMStoreFloat3A((XMFLOAT3A *)v, XMVectorLerp(
		XMLoadFloat3A((XMFLOAT3A *)&key->val), XMLoadFloat3A((XMFLOAT3A *)&nextKey->val),
		(time - key->time) / (nextKey->time - key->time)
	));
}

static inline void
_LerpQuatKey(struct NeQuaternion *q, double time, const struct NeArray *keys)
{
	if (keys->count == 1) {
		memcpy(q, &((struct NeAnimQuatKey *)Rt_ArrayGet(keys, 0))->val, sizeof(*q));
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

	XMStoreFloat4A((XMFLOAT4A *)q, XMQuaternionSlerp(
		XMLoadFloat4A((XMFLOAT4A *)&key->val), XMLoadFloat3A((XMFLOAT3A *)&nextKey->val),
		(time - key->time) / (nextKey->time - key->time)
	));
}

static void
_XformHierarchy(double time, const struct NeSkeleton *skel,
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
		if (ch->positionKeys.count) {
			struct NeVec3 pos;
			_LerpVecKey(&pos, time, &ch->positionKeys);

			xform = XMMatrixMultiply(xform, XMMatrixTranslation(pos.x, pos.y, pos.z));
		}

		if (ch->rotationKeys.count) {
			struct NeQuaternion rot;
			_LerpQuatKey(&rot, time, &ch->rotationKeys);

			xform = XMMatrixMultiply(xform, XMMatrixRotationQuaternion(XMLoadFloat4A((XMFLOAT4A *)&rot)));
		}

		if (ch->scalingKeys.count) {
			struct NeVec3 scale;
			_LerpVecKey(&scale, time, &ch->scalingKeys);
			
			xform = XMMatrixMultiply(xform, XMMatrixScaling(scale.x, scale.y, scale.z));
		}
	} else {
		xform = XMLoadFloat4x4A((XMFLOAT4X4A *)&n->xform);
	}

	XMMATRIX gXform = XMMatrixMultiply(XMLoadFloat4x4A((XMFLOAT4X4A *)&skel->globalInverseTransform),
		XMMatrixMultiply(parentXform, xform));

	const struct NeBone *b = NULL;
	Rt_ArrayForEach(b, &skel->bones, const struct NeBone *) {
		if (b->hash != n->hash)
			continue;
		
		XMStoreFloat4x4A((XMFLOAT4X4A *)&transforms[miwa_rtafei],
			XMMatrixMultiply(gXform, XMLoadFloat4x4A((XMFLOAT4X4A *)&b->offset)));

		break;
	}
	
	struct NeSkeletonNode *child;
	Rt_ArrayForEachPtr(child, &n->children, struct NeSkeletonNode *)
		_XformHierarchy(time, skel, child, xform, ac, transforms);
}

/* NekoEngine
 *
 * Skeleton.c
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
