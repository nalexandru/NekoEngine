#include <Script/Script.h>
#include <Render/Context.h>
#include <Render/RenderPass.h>

// Context

static int _S_Re_BeginDrawCommandBuffer(lua_State *vm) { Re_BeginDrawCommandBuffer(); return 0; }
static int _S_Re_BeginComputeCommandBuffer(lua_State *vm) { Re_BeginComputeCommandBuffer(); return 0; }
static int _S_Re_BeginTransferCommandBuffer(lua_State *vm) { Re_BeginTransferCommandBuffer(); return 0; }
static int _S_Re_EndCommandBuffer(lua_State *vm) { Re_EndCommandBuffer(); return 0; }

static int
_S_Re_BindPipeline(lua_State *vm)
{
	Re_BindPipeline(luaL_checkudata(vm, 1, NULL));
	return 0;
}

static int
_S_Re_BindDescriptorSets(lua_State *vm)
{
	//Re_BindDescriptorSets(<#struct PipelineLayout *layout#>, <#uint32_t firstSet#>, <#uint32_t count#>, <#struct DescriptorSet *sets#>)
	return 0;
}

static int
_S_Re_PushConstants(lua_State *vm)
{
	//Re_PushConstants(<#struct PipelineLayout *layout#>, <#enum ShaderStage stage#>, <#uint32_t size#>, <#const void *data#>)
	return 0;
}

static int
_S_Re_BindIndexBuffer(lua_State *vm)
{
	return 0;
}

static int
_S_Re_ExecuteSecondary(lua_State *vm)
{
	return 0;
}

static int
_S_Re_BeginRenderPass(lua_State *vm)
{
	return 0;
}

static int
_S_Re_EndRenderPass(lua_State *vm)
{
	return 0;
}

static int
_S_Re_SetViewport(lua_State *vm)
{
	return 0;
}

static int
_S_Re_SetScissor(lua_State *vm)
{
	return 0;
}

static int
_S_Re_Draw(lua_State *vm)
{
	Re_Draw((int32_t)luaL_checkinteger(vm, 1),
			(int32_t)luaL_checkinteger(vm, 2),
			(int32_t)luaL_checkinteger(vm, 3),
			(int32_t)luaL_checkinteger(vm, 4));
	return 0;
}

static int
_S_Re_DrawIndexed(lua_State *vm)
{
	Re_DrawIndexed((int32_t)luaL_checkinteger(vm, 1),
				   (int32_t)luaL_checkinteger(vm, 2),
				   (int32_t)luaL_checkinteger(vm, 3),
				   (int32_t)luaL_checkinteger(vm, 4),
				   (int32_t)luaL_checkinteger(vm, 5));
	return 0;
}

static int
_S_Re_DrawIndirect(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DrawIndexedIndirect(lua_State *vm)
{
	return 0;
}

static int
_S_Re_Dispatch(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DispatchIndirect(lua_State *vm)
{
	return 0;
}

static int
_S_Re_TraceRays(lua_State *vm)
{
	return 0;
}

static int
_S_Re_TraceRaysIndirect(lua_State *vm)
{
	return 0;
}

static int
_S_Re_Barrier(lua_State *vm)
{
	return 0;
}

static int
_S_Re_Transition(lua_State *vm)
{
	return 0;
}

static int
_S_Re_CopyBuffer(lua_State *vm)
{
	return 0;
}

static int
_S_Re_CopyImage(lua_State *vm)
{
	return 0;
}

static int
_S_Re_CopyBufferToImage(lua_State *vm)
{
	return 0;
}

static int
_S_Re_CopyImageToBuffer(lua_State *vm)
{
	return 0;
}

static int
_S_Re_Blit(lua_State *vm)
{
	return 0;
}

// Pipeline

static int
_S_Re_CreatePipelineLayout(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DestroyPipelineLayout(lua_State *vm)
{
	return 0;
}

static int
_S_Re_GraphicsPipeline(lua_State *vm)
{
	return 0;
}

static int
_S_Re_ComputePipeline(lua_State *vm)
{
	return 0;
}

static int
_S_Re_RayTracingPipeline(lua_State *vm)
{
	return 0;
}

// Texture
static int
_S_Re_CreateTexture(lua_State *vm)
{
	return 0;
}

static int
_S_Re_TextureDesc(lua_State *vm)
{
	return 0;
}

static int
_S_Re_TextureLayout(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DestroyTexture(lua_State *vm)
{
	return 0;
}

// Buffer
static int
_S_Re_CreateBuffer(lua_State *vm)
{
	return 0;
}

static int
_S_Re_UpdateBuffer(lua_State *vm)
{
	return 0;
}

static int
_S_Re_BufferDesc(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DestroyBuffer(lua_State *vm)
{
	return 0;
}

// Framebuffer
static int
_S_Re_CreateFramebuffer(lua_State *vm)
{
	return 0;
}

static int
_S_Re_SetAttachment(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DestroyFramebuffer(lua_State *vm)
{
	return 0;
}

// RenderPass
static int
_S_Re_CreateRenderPass(lua_State *vm)
{
	/*struct RenderPassDesc desc =
	{
		.attachmentCount = 1,
		.attachments = &atDesc,
	};*/
	
	struct RenderPass *rp = Re_CreateRenderPass(NULL);
	
	lua_pushlightuserdata(vm, rp);
	
	return 1;
}

static int
_S_Re_DestroyRenderPass(lua_State *vm)
{
	return 0;
}

// Swapchain
static int
_S_Re_AcquireNextImage(lua_State *vm)
{
	return 0;
}

static int
_S_Re_SwapchainFormat(lua_State *vm)
{
	return 0;
}

static int
_S_Re_SwapchainTexture(lua_State *vm)
{
	return 0;
}

static int
_S_Re_Present(lua_State *vm)
{
	return 0;
}

// Descriptor Set
static int
_S_Re_CreateDescriptorSetLayout(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DestroyDescriptorSetLayout(lua_State *vm)
{
	return 0;
}

static int
_S_Re_CreateDescriptorSet(lua_State *vm)
{
	return 0;
}

static int
_S_Re_WriteDescriptorSet(lua_State *vm)
{
	return 0;
}

static int
_S_Re_DestroyDescriptorSet(lua_State *vm)
{
	return 0;
}

// Shader
static int
_S_Re_GetShader(lua_State *vm)
{
	struct Shader *s = Re_GetShader(luaL_checkstring(vm, 1));
	lua_pushlightuserdata(vm, s);
	return 1;
}

void
SIface_OpenRender(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		// Render Context
		{ "BeginDrawCommandBuffer", _S_Re_BeginDrawCommandBuffer },
		{ "BeginComputeCommandBuffer", _S_Re_BeginComputeCommandBuffer },
		{ "BeginTransferCommandBuffer", _S_Re_BeginTransferCommandBuffer },
		{ "EndCommandBuffer", _S_Re_EndCommandBuffer },
		{ "BindPipeline", _S_Re_BindPipeline },
		{ "BindDescriptorSets", _S_Re_BindDescriptorSets },
		{ "PushConstants", _S_Re_PushConstants },
		{ "BindIndexBuffer",  _S_Re_BindIndexBuffer },
		{ "ExecuteSecondary", _S_Re_ExecuteSecondary },
		{ "BeginRenderPass", _S_Re_BeginRenderPass },
		{ "EndRenderPass", _S_Re_EndRenderPass },
		{ "SetViewport", _S_Re_SetViewport },
		{ "SetScissor", _S_Re_SetScissor },
		{ "Draw", _S_Re_Draw },
		{ "DrawIndexed", _S_Re_DrawIndexed },
		{ "DrawIndirect", _S_Re_DrawIndirect },
		{ "DrawIndexedIndirect", _S_Re_DrawIndexedIndirect },
		{ "Dispatch", _S_Re_Dispatch },
		{ "DispatchIndirect", _S_Re_DispatchIndirect },
		{ "TraceRays", _S_Re_TraceRays },
		{ "TraceRaysIndirect", _S_Re_TraceRaysIndirect },
		{ "Barrier", _S_Re_Barrier },
		{ "Transition", _S_Re_Transition },
		{ "CopyBuffer", _S_Re_CopyBuffer },
		{ "CopyImage", _S_Re_CopyImage },
		{ "CopyBufferToImage", _S_Re_CopyBufferToImage },
		{ "CopyImageToBuffer", _S_Re_CopyImageToBuffer },
		{ "Blit", _S_Re_Blit },
		
		// Pipeline
		{ "CreatePipelineLayout", _S_Re_CreatePipelineLayout },
		{ "DestroyPipelineLayout", _S_Re_DestroyPipelineLayout },
		{ "GraphicsPipeline", _S_Re_GraphicsPipeline },
		{ "ComputePipeline", _S_Re_ComputePipeline },
		{ "RayTracingPipeline", _S_Re_RayTracingPipeline },

		// Texture
		{ "CreateTexture", _S_Re_CreateTexture },
		{ "TextureDesc", _S_Re_TextureDesc },
		{ "TextureLayout", _S_Re_TextureLayout },
		{ "DestroyTexture", _S_Re_DestroyTexture },
		
		// Buffer
		{ "CreateBuffer", _S_Re_CreateBuffer },
		{ "UpdateBuffer", _S_Re_UpdateBuffer },
		{ "BufferDesc", _S_Re_BufferDesc },
		{ "DestroyBuffer", _S_Re_DestroyBuffer },

		// Framebuffer
		{ "CreateFramebuffer", _S_Re_CreateFramebuffer },
		{ "SetAttachment", _S_Re_SetAttachment },
		{ "DestroyFramebuffer", _S_Re_DestroyFramebuffer },
		
		// Render Pass
		{ "CreateRenderPass", _S_Re_CreateRenderPass },
		{ "DestroyRenderPass", _S_Re_DestroyRenderPass },
		
		// Swapchain
		{ "AcquireNextImage", _S_Re_AcquireNextImage },
		{ "SwapchainFormat", _S_Re_SwapchainFormat },
		{ "SwapchainTexture", _S_Re_SwapchainTexture },
		{ "Present", _S_Re_Present },
		
		// Swapchain
		{ "CreateDescriptorSetLayout", _S_Re_CreateDescriptorSetLayout },
		{ "DestroyDescriptorSetLayout", _S_Re_DestroyDescriptorSetLayout },
		{ "CreateDescriptorSet", _S_Re_CreateDescriptorSet },
		{ "WriteDescriptorSet", _S_Re_WriteDescriptorSet },
		{ "DestroyDescriptorSet", _S_Re_DestroyDescriptorSet },
		
		// Shader
		{ "GetShader", _S_Re_GetShader }
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Re");
}
