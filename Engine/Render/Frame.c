#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Engine/ECSystem.h>
#include <Runtime/Runtime.h>

#include <Engine/Resource.h>

#include <Render/Model.h>

uint32_t Re_frameId = 0;

void
Re_RenderFrame(void)
{
	static struct
	{
		uint32_t vertexBuffer;
		uint32_t __padding;
		uint64_t materialAddress;
	} constants;

	static struct CollectDrawablesArgs args = { 0 };

	Re_TransferMaterials();

	if (!args.arrays) {
		args.arrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct Array), MH_Render);

		for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
			Rt_InitArray(&args.arrays[i], 10, sizeof(struct Drawable), MH_Render);
	} else {
		args.nextArray = 0;
	}

	E_ExecuteSystemS(Scn_activeScene, RE_COLLECT_DRAWABLES, &args);

	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_ResetContext(Re_contexts[i]);
	Re_DestroyResources();
	Sys_ResetHeap(MH_Frame);

	Re_BeginDrawCommandBuffer();

	struct FramebufferAttachmentDesc fbAtDesc =
	{
		.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
		.format = Re_SwapchainFormat(Re_swapchain)
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
		.attachments = &fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = Re_MaterialRenderPassDesc
	};
	struct Framebuffer *fb = Re_CreateFramebuffer(&fbDesc);
	Re_SetAttachment(fb, 0, Re_SwapchainTexture(Re_swapchain, image));

	Re_CmdBeginRenderPass(Re_MaterialRenderPassDesc, fb, RENDER_COMMANDS_INLINE);

	Re_Destroy(fb);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		struct Array *drawables = &args.arrays[i];
		struct Drawable *d = NULL;

		Rt_ArrayForEach(d, drawables) {
			Re_CmdBindPipeline(d->material->pipeline);

			Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
			Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);

			constants.vertexBuffer = d->vertexBuffer;
			constants.materialAddress = d->materialAddress;

			Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);

			Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
		}

		Rt_ClearArray(drawables, false);
	}

	Re_CmdEndRenderPass();
	Re_EndCommandBuffer();

	Re_Present(Re_swapchain, image);

	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}


/* TODO: multithreaded rendering
 * static inline void
_DrawSecondary(void *image, struct RenderConstants *constants)
{
	CommandBufferHandle sec = Re_BeginSecondary(_rpd);

	Re_BindPipeline(_pipeline);

	Re_SetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_SetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	Re_BindIndexBuffer(_indexBuffer, 0, IT_UINT_16);
	Re_PushConstants(SS_ALL, sizeof(*constants), constants);
	Re_DrawIndexed(6, 1, 0, 0, 0);

	Re_EndCommandBuffer();

	Re_BeginDrawCommandBuffer();

	struct FramebufferAttachmentDesc fbAtDesc =
	{
		.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
		.format = Re_SwapchainFormat(Re_swapchain)
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
		.attachments = &fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = _rpd
	};
	_fb = Re_CreateFramebuffer(&fbDesc);
	Re_SetAttachment(_fb, 0, Re_SwapchainTexture(Re_swapchain, image));

	Re_BeginRenderPass(_rpd, _fb, RENDER_COMMANDS_SECONDARY_COMMAND_BUFFERS);

	Re_Destroy(_fb);

	Re_ExecuteSecondary(&sec, 1);

	Re_EndRenderPass();
	Re_EndCommandBuffer();
}*/
