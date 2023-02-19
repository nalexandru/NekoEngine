#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Engine/Resource.h>
#include <Animation/Animation.h>
#include <Scene/Components.h>

// FIXME
#include <System/PlatformDetect.h>

static bool _InitAnimator(struct NeAnimator *a, const void **args);
static void _TermAnimator(struct NeAnimator *a);

E_REGISTER_COMPONENT(ANIMATOR_COMP, struct NeAnimator, 16, _InitAnimator, _TermAnimator)

static bool
_InitAnimator(struct NeAnimator *a, const void **args)
{
	a->clip = E_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Loop", len)) {
			a->loop = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "OneShot", len)) {
			a->oneShot = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Play", len)) {
			a->playing = !strncmp(*(++args), "true", 4);
		} else if (!strncmp(arg, "Clip", len)) {
			a->clip = E_LoadResource(*(++args), RES_ANIMATION_CLIP);
/*		 } else if (!strncmp(arg, "Aperture", len)) {
			cam->zFar = (float)atof(*(++args));*/
		}
	}
	
	a->skel = Sys_Alloc(sizeof(*a->skel), 1, MH_Scene);

	return true;
}

E_SYSTEM(ANIM_BUILD_SKELETON, ECSYS_GROUP_MANUAL, 0, false, void, 2, ANIMATOR_COMP, MODEL_RENDER_COMP)
{
	struct NeAnimator *a = comp[0];
	struct NeModelRender *mr = comp[1];

	const struct NeModel *m = E_ResourcePtr(mr->model);
	if (!m)
		return;

	Anim_InitSkeleton(a->skel, &m->skeleton.bones, &m->skeleton.nodes, &m->skeleton.globalInverseTransform);

	a->boneBufferSize = sizeof(struct NeMatrix) * m->skeleton.bones.count;
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

	a->transforms = Sys_Alloc(sizeof(*a->transforms), m->skeleton.bones.count, MH_Scene);
//	for (size_t i = 0; i < m->skeleton.bones.count; ++i)
//		M_Identity(&a->transforms[i]);
		//M_Copy(&a->transforms[i], &((struct NeBone *)Rt_ArrayGet(&m->skeleton.bones, i))->offset);
}

E_SYSTEM(ANIM_UPDATE_ANIMATOR, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_SKINNING, true, void, 1, ANIMATOR_COMP)
{
	struct NeAnimator *a = comp[0];
	const struct NeAnimationClip *ac = E_ResourcePtr(a->clip);

	if (a->playing && ac) {
		a->time += E_deltaTime;
		if (a->time > ac->duration) {
			if (a->loop) {
				a->time = 0.0;
			} else if (a->oneShot) {
				a->clip = a->prevClip;
				a->time = a->prevTime;
				ac = E_ResourcePtr(a->clip);
				if (!ac)
					return;
			} else {
				a->playing = false;
			}
		}
		
		Anim_UpdateSkeleton(a->skel, a->time, ac, a->transforms);
	}

#ifdef SYS_PLATFORM_APPLE
	Re_BeginTransferCommandBuffer();
	Re_CmdUpdateBuffer(a->boneBuffer, Re_frameId * a->boneBufferSize, a->transforms, a->boneBufferSize);
	NeCommandBufferHandle cmdBuff = Re_EndCommandBuffer();
	Re_QueueTransfer(cmdBuff, NULL, NULL);
#endif
	
	a->dirty = true;
}

E_SYSTEM(ANIM_COMPUTE_SKINNING, ECSYS_GROUP_MANUAL, ECSYS_PRI_SKINNING, true, void, 2, ANIMATOR_COMP, MODEL_RENDER_COMP)
{
	struct NeAnimator *a = comp[0];
	struct NeModelRender *mr = comp[1];
	struct NeSkinningData constants;

	if (!a->dirty)
		return;

#ifndef SYS_PLATFORM_APPLE
	Re_CmdUpdateBuffer(a->boneBuffer, Re_frameId * a->boneBufferSize, a->transforms, a->boneBufferSize);
#endif
	
	struct NeModel *mdl = E_ResourcePtr(mr->model);
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
_TermAnimator(struct NeAnimator *a)
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
 * Animator.c
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
