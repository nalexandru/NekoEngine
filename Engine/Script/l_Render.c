#include <Script/Script.h>
#include <Render/Render.h>

#include "Interface.h"

// Context

SIF_FUNC(BeginDrawCommandBuffer) { Re_BeginDrawCommandBuffer(); return 0; }
SIF_FUNC(BeginComputeCommandBuffer) { Re_BeginComputeCommandBuffer(); return 0; }
SIF_FUNC(BeginTransferCommandBuffer) { Re_BeginTransferCommandBuffer(); return 0; }
SIF_FUNC(EndCommandBuffer) { Re_EndCommandBuffer(); return 0; }

SIF_FUNC(BindPipeline)
{
	Re_CmdBindPipeline(luaL_checkudata(vm, 1, NULL));
	return 0;
}

SIF_FUNC(PushConstants)
{
	//Re_PushConstants(<#struct PipelineLayout *layout#>, <#enum ShaderStage stage#>, <#uint32_t size#>, <#const void *data#>)
	return 0;
}

SIF_FUNC(BindIndexBuffer)
{
	return 0;
}

SIF_FUNC(ExecuteSecondary)
{
	return 0;
}

SIF_FUNC(BeginRenderPass)
{
	return 0;
}

SIF_FUNC(EndRenderPass)
{
	return 0;
}

SIF_FUNC(SetViewport)
{
	return 0;
}

SIF_FUNC(SetScissor)
{
	return 0;
}

SIF_FUNC(Draw)
{
	Re_CmdDraw((int32_t)luaL_checkinteger(vm, 1),
			(int32_t)luaL_checkinteger(vm, 2),
			(int32_t)luaL_checkinteger(vm, 3),
			(int32_t)luaL_checkinteger(vm, 4));
	return 0;
}

SIF_FUNC(DrawIndexed)
{
	Re_CmdDrawIndexed((int32_t)luaL_checkinteger(vm, 1),
				   (int32_t)luaL_checkinteger(vm, 2),
				   (int32_t)luaL_checkinteger(vm, 3),
				   (int32_t)luaL_checkinteger(vm, 4),
				   (int32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(DrawIndirect)
{
	return 0;
}

SIF_FUNC(DrawIndexedIndirect)
{
	return 0;
}

SIF_FUNC(Dispatch)
{
	return 0;
}

SIF_FUNC(DispatchIndirect)
{
	return 0;
}

SIF_FUNC(TraceRays)
{
	return 0;
}

SIF_FUNC(TraceRaysIndirect)
{
	return 0;
}

SIF_FUNC(Barrier)
{
	return 0;
}

SIF_FUNC(Transition)
{
	return 0;
}

SIF_FUNC(CopyBuffer)
{
	return 0;
}

SIF_FUNC(CopyImage)
{
	return 0;
}

SIF_FUNC(CopyBufferToImage)
{
	return 0;
}

SIF_FUNC(CopyImageToBuffer)
{
	return 0;
}

SIF_FUNC(Blit)
{
	return 0;
}

// Pipeline
SIF_FUNC(GraphicsPipeline)
{
	return 0;
}

SIF_FUNC(ComputePipeline)
{
	return 0;
}

SIF_FUNC(RayTracingPipeline)
{
	return 0;
}

// Texture
SIF_FUNC(CreateTexture)
{
	return 0;
}

SIF_FUNC(TextureDesc)
{
	return 0;
}

SIF_FUNC(TextureLayout)
{
	return 0;
}

SIF_FUNC(DestroyTexture)
{
	return 0;
}

// Buffer
SIF_FUNC(CreateBuffer)
{
	return 0;
}

SIF_FUNC(UpdateBuffer)
{
	return 0;
}

SIF_FUNC(BufferDesc)
{
	return 0;
}

SIF_FUNC(DestroyBuffer)
{
	return 0;
}

// Framebuffer
SIF_FUNC(CreateFramebuffer)
{
	return 0;
}

SIF_FUNC(SetAttachment)
{
	return 0;
}

SIF_FUNC(DestroyFramebuffer)
{
	return 0;
}

// RenderPass
SIF_FUNC(CreateRenderPassDesc)
{
	/*struct RenderPassDesc desc =
	{
		.attachmentCount = 1,
		.attachments = &atDesc,
	};*/
	
	struct RenderPassDesc *rp = Re_CreateRenderPassDesc(NULL, 0, NULL);
	
	lua_pushlightuserdata(vm, rp);
	
	return 1;
}

SIF_FUNC(DestroyRenderPassDesc)
{
	return 0;
}

// Swapchain
SIF_FUNC(AcquireNextImage)
{
	return 0;
}

SIF_FUNC(SwapchainFormat)
{
	return 0;
}

SIF_FUNC(SwapchainTexture)
{
	return 0;
}

SIF_FUNC(Present)
{
	return 0;
}

// Shader
SIF_FUNC(GetShader)
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
		SIF_REG(BeginDrawCommandBuffer),
		SIF_REG(BeginComputeCommandBuffer),
		SIF_REG(BeginTransferCommandBuffer),
		SIF_REG(EndCommandBuffer),
		SIF_REG(BindPipeline),

		// Render Context
		SIF_REG(PushConstants),
		SIF_REG(BindIndexBuffer),
		SIF_REG(ExecuteSecondary),
		SIF_REG(BeginRenderPass),
		SIF_REG(EndRenderPass),
		SIF_REG(SetViewport),
		SIF_REG(SetScissor),
		SIF_REG(Draw),
		SIF_REG(DrawIndexed),
		SIF_REG(DrawIndirect),
		SIF_REG(DrawIndexedIndirect),
		SIF_REG(Dispatch),
		SIF_REG(DispatchIndirect),
		SIF_REG(TraceRays),
		SIF_REG(TraceRaysIndirect),
		SIF_REG(Barrier),
		SIF_REG(Transition),
		SIF_REG(CopyBuffer),
		SIF_REG(CopyImage),
		SIF_REG(CopyBufferToImage),
		SIF_REG(CopyImageToBuffer),
		SIF_REG(Blit),
		
		// Pipeline
		SIF_REG(GraphicsPipeline),
		SIF_REG(ComputePipeline),
		SIF_REG(RayTracingPipeline),

		// Texture
		SIF_REG(CreateTexture),
		SIF_REG(TextureDesc),
		SIF_REG(TextureLayout),
		SIF_REG(DestroyTexture),
		
		// Buffer
		SIF_REG(CreateBuffer),
		SIF_REG(UpdateBuffer),
		SIF_REG(BufferDesc),
		SIF_REG(DestroyBuffer),

		// Framebuffer
		SIF_REG(CreateFramebuffer),
		SIF_REG(SetAttachment),
		SIF_REG(DestroyFramebuffer),
		
		// Render Pass
		SIF_REG(CreateRenderPassDesc),
		SIF_REG(DestroyRenderPassDesc),
		
		// Swapchain
		SIF_REG(AcquireNextImage),
		SIF_REG(SwapchainFormat),
		SIF_REG(SwapchainTexture),
		SIF_REG(Present),
		
		// Shader
		SIF_REG(GetShader),

		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Re");
}
