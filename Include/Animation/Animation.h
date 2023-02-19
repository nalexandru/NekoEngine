#ifndef _NE_ANIMATION_ANIMATION_H_
#define _NE_ANIMATION_ANIMATION_H_

#include <Render/Types.h>
#include <Animation/Clip.h>
#include <Animation/Skeleton.h>
#include <Animation/Animator.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ANIM_BUILD_SKELETON		"Anim_BuildSkeleton"
#define ANIM_UPDATE_ANIMATOR	"Anim_UpdateAnimator"
#define ANIM_COMPUTE_SKINNING	"Anim_ComputeSkinning"

struct NeSkinningData
{
	uint64_t bones;
	uint64_t weights;
	uint64_t source;
	uint64_t destination;
};

extern struct NePipeline *Anim_skinningPipeline;

bool Anim_InitAnimationSystem(void);
void Anim_TermAnimationSystem(void);

#ifdef __cplusplus
}
#endif

#endif /* _NE_ANIMATION_ANIMATION_H_ */

/* NekoEngine
 *
 * Animation.h
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
