#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Engine/ECSystem.h>
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
	///////////////////////////////////
	if (!Re_activeGraph) {
		Re_activeGraph = Re_CreateGraph();

		Re_AddPass(Re_activeGraph, &RP_depthPrePass);
		Re_AddPass(Re_activeGraph, &RP_lightCulling);
		Re_AddPass(Re_activeGraph, &RP_opaque);
		Re_AddPass(Re_activeGraph, &RP_sky);
		Re_AddPass(Re_activeGraph, &RP_transparent);
		Re_AddPass(Re_activeGraph, &RP_ui);
	}
	///////////////////////////////////

	Scn_StartDrawableCollection(Scn_activeScene, Scn_activeCamera);

	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_ResetContext(Re_contexts[i]);
	Re_DestroyResources();
	Sys_ResetHeap(MH_Frame);

	Scn_StartDataUpdate(Scn_activeScene, Scn_activeCamera);
	Re_TransferMaterials();

	Re_BuildGraph(Re_activeGraph, Re_SwapchainTexture(Re_swapchain, image));
	Re_ExecuteGraph(Re_activeGraph);

	Re_Present(Re_swapchain, image, NULL);

	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}
