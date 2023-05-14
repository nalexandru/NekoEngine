#ifndef NE_ANIMATION_SKELETON_H
#define NE_ANIMATION_SKELETON_H

#include <Math/Types.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeSkeletonNode
{
	uint64_t hash;
	struct NeMatrix xform;
	struct NeSkeletonNode *parent;
	struct NeArray children;
	char name[233];
};

struct NeSkeleton
{
	struct NeSkeletonNode *root;
	struct NeArray joints, nodes;
	struct NeMatrix *inverseBind;
	struct NeMatrix inverseTransform;
};

bool Anim_InitSkeleton(struct NeSkeleton *s, const struct NeModel *mdl);
void Anim_UpdateSkeleton(struct NeSkeleton *s, float time, const struct NeAnimationClip *ac, struct NeMatrix *transforms);
void Anim_TermSkeleton(struct NeSkeleton *s);

#ifdef __cplusplus
}
#endif

#endif /* NE_ANIMATION_SKELETON_H */

/* NekoEngine
 *
 * Skeleton.h
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
