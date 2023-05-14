#include <Engine/Job.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

#include "Internal.h"

uint32_t Re_frameId = 0;
struct NeRenderGraph *Re_activeGraph = NULL;

void
Re_RenderFrame(void)
{
	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_ResetContext(Re_contexts[i]);
	Re_DestroyResources();

	Re_ResetTransientHeap();
	Sys_ResetHeap(MH_Frame);

	struct NeTextureDesc desc;
	Re_SwapchainTextureDesc(Re_swapchain, &desc);
	Re_RenderScene(Scn_activeScene, Scn_activeScene->camera, Re_activeGraph, &desc, Re_SwapchainTexture(Re_swapchain, image));

	Re_Present(Re_swapchain, image, NULL);

	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}

void
Re_RenderScene(struct NeScene *scn, NeHandle camHandle, struct NeRenderGraph *graph, const struct NeTextureDesc *desc, struct NeTexture *target)
{
	E_ExecuteSystemGroupS(scn, ECSYS_GROUP_PRE_RENDER_HASH);

	const struct NeCamera *cam = E_ComponentPtr(camHandle);
	Scn_StartDrawableCollection(scn, cam);

	Scn_StartDataUpdate(scn, cam);
	Re_TransferMaterials();

	Re_BuildGraph(Re_activeGraph, desc, target);
	Re_ExecuteGraph(Re_activeGraph);

	E_ExecuteSystemGroupS(scn, ECSYS_GROUP_POST_RENDER_HASH);
}

/* NekoEngine
 *
 * Frame.c
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
