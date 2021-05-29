#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Engine/ECSystem.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>

#include <Engine/Resource.h>

#include <Render/Model.h>

uint32_t Re_frameId = 0;

#include <Script/Script.h>
static void _RenderScript(void);
static void _Cleanup(void);
static struct CollectDrawablesArgs _args = { 0 };
static bool _initialized = false;
static lua_State *vm = NULL;

void
Re_RenderFrame(void)
{
	//_RenderScript(); return;

	/////////////////
	if (!_initialized) {
		atexit(_Cleanup);
		_initialized = true;
	}
	/////////////////
	static struct
	{
		uint64_t vertexAddress;
		uint64_t materialAddress;
		struct mat4 mvp;
	} constants;

	Re_TransferMaterials();

	if (!_args.arrays) {
		_args.arrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct Array), MH_Render);

		for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
			Rt_InitArray(&_args.arrays[i], 10, sizeof(struct Drawable), MH_Render);
	} else {
		_args.nextArray = 0;
	}

	m4_mul(&_args.vp, &Scn_activeCamera->projMatrix, &Scn_activeCamera->viewMatrix);
	E_ExecuteSystemS(Scn_activeScene, RE_COLLECT_DRAWABLES, &_args);

	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_ResetContext(Re_contexts[i]);
	Re_DestroyResources();
	Sys_ResetHeap(MH_Frame);

	Re_BeginDrawCommandBuffer();

	struct FramebufferAttachmentDesc fbAtDesc[2] =
	{
		{
			.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
			.format = Re_SwapchainFormat(Re_swapchain)
		},
		{
			.usage = TU_DEPTH_STENCIL_ATTACHMENT,
			.format = TF_D32_SFLOAT
		}
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 2,
		.attachments = fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = Re_MaterialRenderPassDesc
	};
	struct Framebuffer *fb = Re_CreateFramebuffer(&fbDesc);
	Re_SetAttachment(fb, 0, Re_SwapchainTexture(Re_swapchain, image));
	
	struct TextureCreateInfo depthInfo =
	{
		.desc = {
			.width = *E_screenWidth,
			.height = *E_screenHeight,
			.depth = 1,
			.type = TT_2D,
			.usage = TU_DEPTH_STENCIL_ATTACHMENT,
			.format = TF_D32_SFLOAT,
			.arrayLayers = 1,
			.mipLevels = 1,
			.gpuOptimalTiling = true,
			.memoryType = MT_GPU_LOCAL
		}
	};
	struct Texture *depthTexture = Re_CreateTransientTexture(&depthInfo, 0);
	Re_SetAttachment(fb, 1, depthTexture);

	Re_Destroy(depthTexture);
	Re_Destroy(fb);

	Re_CmdBeginRenderPass(Re_MaterialRenderPassDesc, fb, RENDER_COMMANDS_INLINE);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		struct Array *drawables = &_args.arrays[i];
		struct Drawable *d = NULL;

		if (!drawables->count)
			continue;

		Rt_ArrayForEach(d, drawables) {
			Re_CmdBindPipeline(d->material->pipeline);

			Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
			Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);

			constants.vertexAddress = d->vertexAddress;
			constants.materialAddress = d->materialAddress;
			m4_copy(&constants.mvp, &d->mvp);

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

void
_RenderScript(void)
{
	static struct
	{
		uint64_t vertexAddress;
		uint64_t materialAddress;
		struct mat4 mvp;
	} constants;

	if (!_initialized) {
		vm = Sc_CreateVM(true);
		Sc_LoadScriptFile(vm, "/Scripts/Frame.lua");

		_initialized = true;

		atexit(_Cleanup);
	}

	Re_TransferMaterials();

	if (!_args.arrays) {
		_args.arrays = Sys_Alloc(E_JobWorkerThreads(), sizeof(struct Array), MH_Render);

		for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
			Rt_InitArray(&_args.arrays[i], 10, sizeof(struct Drawable), MH_Render);
	} else {
		_args.nextArray = 0;
	}

	m4_mul(&_args.vp, &Scn_activeCamera->projMatrix, &Scn_activeCamera->viewMatrix);
	E_ExecuteSystemS(Scn_activeScene, RE_COLLECT_DRAWABLES, &_args);

	void *image = Re_AcquireNextImage(Re_swapchain);
	if (image == RE_INVALID_IMAGE)
		return;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Re_ResetContext(Re_contexts[i]);
	Re_DestroyResources();
	Sys_ResetHeap(MH_Frame);

	/*for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		struct Array *drawables = &args.arrays[i];
		struct Drawable *d = NULL;

		Rt_ArrayForEach(d, drawables) {
			// prepare lua drawables
		}

		Rt_ClearArray(drawables, false);
	}*/

	struct Drawable *d = Rt_ArrayGet(&_args.arrays[0], 0);

	lua_getglobal(vm, "RenderFrame");

	lua_pushlightuserdata(vm, image);

	lua_createtable(vm, 0, 6);

	lua_pushlightuserdata(vm, d->material->pipeline);
	lua_setfield(vm, -2, "pipeline");

	lua_pushinteger(vm, d->indexBuffer);
	lua_setfield(vm, -2, "indexBuffer");

	lua_pushinteger(vm, d->indexType);
	lua_setfield(vm, -2, "indexType");

	constants.vertexAddress = d->vertexAddress;
	constants.materialAddress = d->materialAddress;
	m4_copy(&constants.mvp, &d->mvp);
	lua_pushlightuserdata(vm, &constants);
	lua_setfield(vm, -2, "constants");

	lua_pushinteger(vm, d->indexCount);
	lua_setfield(vm, -2, "indexCount");

	lua_pushinteger(vm, d->firstIndex);
	lua_setfield(vm, -2, "firstIndex");

	if (lua_pcall(vm, 2, 0, 0) && lua_gettop(vm)) {
		Sys_LogEntry(L"FF", LOG_CRITICAL, L"FATAL: %hs", lua_tostring(vm, -1));
	}

	Rt_ClearArray(&_args.arrays[0], false);

	Re_Present(Re_swapchain, image);

	Re_frameId = (Re_frameId + 1) % RE_NUM_FRAMES;
}

static void
_Cleanup(void)
{
	if (vm)
		lua_close(vm);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
		Rt_TermArray(&_args.arrays[i]);
	Sys_Free(_args.arrays);
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
