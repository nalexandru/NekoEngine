#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Engine/Resource.h>
#include <Animation/Animation.h>
#include <Scene/Components.h>
#include <Math/Math.h>

// FIXME
#include <System/PlatformDetect.h>

static bool InitAnimator(struct NeAnimator *a, const char **args);
static void TermAnimator(struct NeAnimator *a);

NE_REGISTER_COMPONENT(NE_ANIMATOR, struct NeAnimator, 16, InitAnimator, NULL, TermAnimator)

static bool
InitAnimator(struct NeAnimator *a, const char **args)
{
	a->clip = NE_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "Loop", len)) {
			a->loop = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "OneShot", len)) {
			a->oneShot = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Play", len)) {
			a->playing = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Clip", len)) {
			a->clip = E_LoadResource(*(++args), RES_ANIMATION_CLIP);
		}
	}

	a->skel = (struct NeSkeleton *)Sys_Alloc(sizeof(*a->skel), 1, MH_Scene);

	return true;
}

NE_SYSTEM(ANIM_BUILD_SKELETON, ECSYS_GROUP_MANUAL, 0, false, void, 2, NE_ANIMATOR, NE_MODEL_RENDER)
{
	struct NeAnimator *a = (struct NeAnimator *)comp[0];
	struct NeModelRender *mr = (struct NeModelRender *)comp[1];

	const struct NeModel *m = (const struct NeModel *)E_ResourcePtr(mr->model);
	if (!m)
		return;

	Anim_InitSkeleton(a->skel, m);

	a->boneBufferSize = sizeof(struct NeMatrix) * m->skeleton.joints.count;
	struct NeBufferCreateInfo bci = 
	{
		.desc = {
			.size = a->boneBufferSize * RE_NUM_FRAMES,
			.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER,
			.memoryType = MT_GPU_LOCAL
		}
	};
	Re_CreateBuffer(&bci, &a->boneBuffer);

	const struct NeBufferDesc *desc = Re_BufferDesc(m->gpu.vertexBuffer);
	memcpy(&bci.desc, desc, sizeof(bci.desc));
	Re_CreateBuffer(&bci, &a->vertexBuffer);

	a->transforms = (struct NeMatrix *)Sys_Alloc(sizeof(*a->transforms), m->skeleton.joints.count, MH_Scene);
	if (a->clip != NE_INVALID_HANDLE) {
		Anim_UpdateSkeleton(a->skel, 0, (const struct NeAnimationClip *)E_ResourcePtr(a->clip), a->transforms);
	} else {
		for (size_t i = 0; i < m->skeleton.joints.count; ++i)
			M_Store(&a->transforms[i], XMMatrixIdentity());
	}
}

// FIXME: the Metal backend crashes if this is multi-threaded
NE_SYSTEM(ANIM_UPDATE_ANIMATOR, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_SKINNING, true, void, 1, NE_ANIMATOR)
{
	struct NeAnimator *a = (struct NeAnimator *)comp[0];
	const struct NeAnimationClip *ac = (const struct NeAnimationClip *)E_ResourcePtr(a->clip);

	if (a->playing && ac) {
		a->time += (float)E_deltaTime;
		if (a->time > ac->duration) {
			if (a->loop) {
				a->time = 0.0;
			} else if (a->oneShot) {
				a->clip = a->prevClip;
				a->time = a->prevTime;
				ac = (const struct NeAnimationClip *)E_ResourcePtr(a->clip);
				if (!ac)
					return;
			} else {
				a->playing = false;
			}
		}

		Anim_UpdateSkeleton(a->skel, a->time, ac, a->transforms);
	}

#ifdef SYS_PLATFORM_APPLE
	Re_BeginTransferCommandBuffer(nullptr);
	Re_CmdUpdateBuffer(a->boneBuffer, Re_frameId * a->boneBufferSize, a->transforms, a->boneBufferSize);
	NeCommandBufferHandle cmdBuff = Re_EndCommandBuffer();
	Re_QueueTransfer(cmdBuff, nullptr);
#endif

	a->dirty = true;
}

NE_SYSTEM(ANIM_COMPUTE_SKINNING, ECSYS_GROUP_MANUAL, ECSYS_PRI_SKINNING, true, void, 2, NE_ANIMATOR, NE_MODEL_RENDER)
{
	struct NeAnimator *a = (struct NeAnimator *)comp[0];
	struct NeModelRender *mr = (struct NeModelRender *)comp[1];
	struct NeSkinningData constants;

	if (!a->dirty)
		return;

#ifndef SYS_PLATFORM_APPLE
	Re_CmdUpdateBuffer(a->boneBuffer, Re_frameId * a->boneBufferSize, a->transforms, a->boneBufferSize);
#endif

	const struct NeModel *mdl = (const struct NeModel *)E_ResourcePtr(mr->model);
	constants.bones = Re_BufferAddress(a->boneBuffer, Re_frameId * a->boneBufferSize);
	constants.weights = Re_BufferAddress(mdl->gpu.vertexWeightBuffer, 0);
	constants.source = Re_BufferAddress(mdl->gpu.vertexBuffer, 0);
	constants.destination = Re_BufferAddress(a->vertexBuffer, 0);
	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	Re_CmdDispatch(mdl->vertexCount, 1, 1);

	a->dirty = false;
	mr->vertexBuffer = a->vertexBuffer;
}

static void
TermAnimator(struct NeAnimator *a)
{
	E_UnloadResource(a->clip);

	if (!a->skel)
		return;

	Anim_TermSkeleton(a->skel);
	Re_Destroy(a->boneBuffer);
	Re_Destroy(a->vertexBuffer);

	Sys_Free(a->transforms);
	Sys_Free(a->skel);
}

/* NekoEngine
 *
 * Animator.cxx
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
