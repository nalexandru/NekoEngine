#include <System/Log.h>
#include <System/Memory.h>
#include <Script/Script.h>
#include <Render/Render.h>
#include <Render/RayTracing.h>

#include "Interface.h"

// Context

SIF_FUNC(BeginDrawCommandBuffer) { Re_BeginDrawCommandBuffer(); return 0; }
SIF_FUNC(BeginComputeCommandBuffer) { Re_BeginComputeCommandBuffer(); return 0; }
SIF_FUNC(BeginTransferCommandBuffer) { Re_BeginTransferCommandBuffer(); return 0; }
SIF_FUNC(EndCommandBuffer) { Re_EndCommandBuffer(); return 0; }

SIF_FUNC(CmdBindPipeline)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	Re_CmdBindPipeline(lua_touserdata(vm, 1));

	return 0;
}

SIF_FUNC(CmdPushConstants)
{
	if (!lua_islightuserdata(vm, 3))
		luaL_argerror(vm, 3, "");

	Re_CmdPushConstants((uint32_t)luaL_checkinteger(vm, 1), (uint32_t)luaL_checkinteger(vm, 2), lua_touserdata(vm, 3));

	return 0;
}

SIF_FUNC(CmdBindIndexBuffer)
{
	Re_CmdBindIndexBuffer((NeBufferHandle)luaL_checkinteger(vm, 1), luaL_checkinteger(vm, 2), (uint32_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(CmdExecuteSecondary)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	Re_CmdExecuteSecondary(lua_touserdata(vm, 1), (uint32_t)luaL_checkinteger(vm, 2));

	return 0;
}

SIF_FUNC(CmdBeginRenderPass)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	Re_CmdBeginRenderPass(lua_touserdata(vm, 1), lua_touserdata(vm, 2), (uint32_t)luaL_checkinteger(vm, 3));

	return 0;
}

SIF_FUNC(CmdEndRenderPass)
{
	Re_CmdEndRenderPass();
	return 0;
}

SIF_FUNC(CmdSetViewport)
{
	Re_CmdSetViewport((float)luaL_checknumber(vm, 1), (float)luaL_checknumber(vm, 2),
					(float)luaL_checknumber(vm, 3), (float)luaL_checknumber(vm, 4),
					(float)luaL_checknumber(vm, 5), (float)luaL_checknumber(vm, 6));
	return 0;
}

SIF_FUNC(CmdSetScissor)
{
	Re_CmdSetScissor((uint32_t)luaL_checkinteger(vm, 1), (uint32_t)luaL_checkinteger(vm, 2),
					 (uint32_t)luaL_checkinteger(vm, 3), (uint32_t)luaL_checkinteger(vm, 4));
	return 0;
}

SIF_FUNC(CmdDraw)
{
	Re_CmdDraw((int32_t)luaL_checkinteger(vm, 1), (int32_t)luaL_checkinteger(vm, 2),
			(int32_t)luaL_checkinteger(vm, 3), (int32_t)luaL_checkinteger(vm, 4));
	return 0;
}

SIF_FUNC(CmdDrawIndexed)
{
	Re_CmdDrawIndexed((int32_t)luaL_checkinteger(vm, 1), (int32_t)luaL_checkinteger(vm, 2),
					(int32_t)luaL_checkinteger(vm, 3), (int32_t)luaL_checkinteger(vm, 4),
					(int32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(CmdDrawIndirect)
{
	return 0;
}

SIF_FUNC(CmdDrawIndexedIndirect)
{
	return 0;
}

SIF_FUNC(CmdDispatch)
{
	Re_CmdDispatch((int32_t)luaL_checkinteger(vm, 1), (int32_t)luaL_checkinteger(vm, 2), (int32_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(CmdDispatchIndirect)
{
	return 0;
}

SIF_FUNC(CmdTraceRays)
{
	return 0;
}

SIF_FUNC(CmdTraceRaysIndirect)
{
	return 0;
}

SIF_FUNC(CmdBarrier)
{
	return 0;
}

SIF_FUNC(CmdTransition)
{
	return 0;
}

SIF_FUNC(CmdCopyBuffer)
{
	Re_CmdCopyBuffer((NeBufferHandle)luaL_checkinteger(vm, 1), luaL_checkinteger(vm, 2),
					(NeBufferHandle)luaL_checkinteger(vm, 3), luaL_checkinteger(vm, 4),
					luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(CmdCopyImage)
{
	return 0;
}

SIF_FUNC(CmdCopyBufferToImage)
{
	return 0;
}

SIF_FUNC(CmdCopyImageToBuffer)
{
	return 0;
}

SIF_FUNC(CmdBlit)
{
	return 0;
}

// Pipeline
SIF_FUNC(GraphicsPipeline)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeGraphicsPipelineDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);

	desc->flags = SIF_INTFIELD(1, "flags");
	desc->stageInfo = SIF_LUSRDATAFIELD(1, "stageInfo");
	desc->renderPassDesc = SIF_LUSRDATAFIELD(1, "renderPassDesc");
	desc->pushConstantSize = SIF_INTFIELD(1, "pushConstantSize");
	desc->attachmentCount = SIF_INTFIELD(1, "attachmentCount");

	// TODO: desc->vertexDesc

	struct NeBlendAttachmentDesc *atDesc = Sys_Alloc(sizeof(*desc->attachments), desc->attachmentCount, MH_Transient);
	desc->attachments = atDesc;

	int t = SIF_GETFIELD(1, "attachments");
	for (lua_Integer i = 0; i < desc->attachmentCount; ++i) {
		lua_rawgeti(vm, t, i + 1);
		int v = lua_gettop(vm);

		atDesc[i].enableBlend = SIF_BOOLFIELD(v, "enableBlend");
		atDesc[i].srcColor = SIF_INTFIELD(v, "srcColor");
		atDesc[i].dstColor = SIF_INTFIELD(v, "dstColor");
		atDesc[i].colorOp = SIF_INTFIELD(v, "colorOp");
		atDesc[i].srcAlpha = SIF_INTFIELD(v, "srcAlpha");
		atDesc[i].dstAlpha = SIF_INTFIELD(v, "dstAlpha");
		atDesc[i].alphaOp = SIF_INTFIELD(v, "alphaOp");
		atDesc[i].writeMask = SIF_INTFIELD(v, "writeMask");

		lua_remove(vm, v);
	}
	SIF_POPFIELD(t);

	desc->depthFormat = SIF_INTFIELD(1, "depthFormat");
	// TODO: desc->name

	lua_pushlightuserdata(vm, Re_GraphicsPipeline(desc));
	return 1;
}

SIF_FUNC(ComputePipeline)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeComputePipelineDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);

	desc->stageInfo = SIF_LUSRDATAFIELD(1, "stageInfo");
	desc->threadsPerThreadgroup.x = SIF_INTFIELD(1, "threadsPerThreadgroupX");
	desc->threadsPerThreadgroup.y = SIF_INTFIELD(1, "threadsPerThreadgroupY");
	desc->threadsPerThreadgroup.z = SIF_INTFIELD(1, "threadsPerThreadgroupZ");
	desc->pushConstantSize = SIF_INTFIELD(1, "pushConstantSize");

	lua_pushlightuserdata(vm, Re_ComputePipeline(desc));
	return 1;
}

SIF_FUNC(RayTracingPipeline)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeRayTracingPipelineDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);

	lua_pushlightuserdata(vm, Re_RayTracingPipeline(desc));
	return 1;
}

// Texture
SIF_FUNC(TextureLayout)
{
	lua_pushinteger(vm, Re_TextureLayout((NeTextureHandle)luaL_checkinteger(vm, 1)));
	return 1;
}

// Buffer
SIF_FUNC(CreateBuffer)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeBufferCreateInfo *bci = Sys_Alloc(sizeof(*bci), 1, MH_Transient);

	bci->desc.size = SIF_INTFIELD(1, "size");
	bci->desc.usage = SIF_INTFIELD(1, "usage");
	bci->desc.memoryType = SIF_INTFIELD(1, "memoryType");

	NeBufferHandle h;
	if (Re_CreateBuffer(bci, &h))
		lua_pushinteger(vm, h);
	else
		lua_pushinteger(vm, -1);

	return 1;
}

SIF_FUNC(MapBuffer)
{
	lua_pushlightuserdata(vm, Re_MapBuffer((NeBufferHandle)luaL_checkinteger(vm, 1)));
	return 1;
}

SIF_FUNC(UnmapBuffer)
{
	Re_UnmapBuffer((NeBufferHandle)luaL_checkinteger(vm, 1));
	return 0;
}

SIF_FUNC(UpdateBuffer)
{
	//Re_UpdateBuffer();
	return 0;
}

SIF_FUNC(DestroyBuffer)
{
	Re_DestroyBuffer((NeBufferHandle)luaL_checkinteger(vm, 1));
	return 0;
}

// Framebuffer
SIF_FUNC(CreateFramebuffer)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeFramebufferDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);

	desc->attachmentCount = SIF_INTFIELD(1, "attachmentCount");
	desc->layers = SIF_INTFIELD(1, "layers");
	desc->height = SIF_INTFIELD(1, "height");
	desc->width = SIF_INTFIELD(1, "width");
	desc->renderPassDesc = SIF_LUSRDATAFIELD(1, "renderPassDesc");
	desc->attachments = Sys_Alloc(sizeof(*desc->attachments), desc->attachmentCount, MH_Transient);

	int t = SIF_GETFIELD(1, "attachments");
	for (lua_Integer i = 0; i < desc->attachmentCount; ++i) {
		lua_rawgeti(vm, t, i + 1);
		int v = lua_gettop(vm);

		desc->attachments[i].usage = SIF_INTFIELD(v, "usage");
		desc->attachments[i].format = SIF_INTFIELD(v, "format");

		lua_remove(vm, v);
	}
	SIF_POPFIELD(t);

	lua_pushlightuserdata(vm, Re_CreateFramebuffer(desc));
	return 1;
}

SIF_FUNC(SetAttachment)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	if (!lua_islightuserdata(vm, 3))
		luaL_argerror(vm, 3, "");

	Re_SetAttachment(lua_touserdata(vm, 1), (uint32_t)luaL_checkinteger(vm, 2), lua_touserdata(vm, 3));

	return 0;
}

SIF_FUNC(DestroyFramebuffer)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeFramebuffer *fb = lua_touserdata(vm, 1);
	Re_Destroy(fb);

	return 0;
}

// RenderPass
SIF_FUNC(CreateRenderPassDesc)
{
	luaL_checktype(vm, 1, LUA_TTABLE);
	uint32_t count = (uint32_t)luaL_checkinteger(vm, 2);

	if (!lua_istable(vm, 3) && !lua_isnil(vm, 3))
		luaL_argerror(vm, 3, "");

	struct NeAttachmentDesc *atDesc = Sys_Alloc(sizeof(*atDesc), count, MH_Transient);

	for (lua_Integer i = 0; i < count; ++i) {
		lua_rawgeti(vm, 1, i + 1);
		int v = lua_gettop(vm);

		atDesc[i].mayAlias = SIF_BOOLFIELD(v, "mayAlias");
		atDesc[i].format = SIF_INTFIELD(v, "format");
		atDesc[i].loadOp = SIF_INTFIELD(v, "loadOp");
		atDesc[i].storeOp = SIF_INTFIELD(v, "storeOp");
		atDesc[i].samples = SIF_INTFIELD(v, "samples");
		atDesc[i].layout = SIF_INTFIELD(v, "layout");
		atDesc[i].initialLayout = SIF_INTFIELD(v, "initialLayout");
		atDesc[i].finalLayout = SIF_INTFIELD(v, "finalLayout");

		// TODO: clearColor

		lua_remove(vm, v);
	}

	struct NeAttachmentDesc *depthDesc = NULL;

	if (lua_istable(vm, 3)) {
		lua_rawget(vm, 3);
		int v = lua_gettop(vm);

		depthDesc = Sys_Alloc(sizeof(*depthDesc), 1, MH_Transient);
		depthDesc->mayAlias = SIF_BOOLFIELD(v, "mayAlias");
		depthDesc->format = SIF_INTFIELD(v, "format");
		depthDesc->loadOp = SIF_INTFIELD(v, "loadOp");
		depthDesc->storeOp = SIF_INTFIELD(v, "storeOp");
		depthDesc->samples = SIF_INTFIELD(v, "samples");
		depthDesc->layout = SIF_INTFIELD(v, "layout");
		depthDesc->initialLayout = SIF_INTFIELD(v, "initialLayout");
		depthDesc->finalLayout = SIF_INTFIELD(v, "finalLayout");
		depthDesc->clearDepth = SIF_FLOATFIELD(v, "clearDepth");
		depthDesc->clearStencil = SIF_INTFIELD(v, "clearStencil");

		lua_remove(vm, v);
	}

	lua_pushlightuserdata(vm, Re_CreateRenderPassDesc(atDesc, count, depthDesc, NULL, 0));
	
	return 1;
}

SIF_FUNC(DestroyRenderPassDesc)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	struct NeRenderPassDesc *rpd = lua_touserdata(vm, 1);
	Re_DestroyRenderPassDesc(rpd);

	return 0;
}

// Swapchain
SIF_LUSRDATA(AcquireNextImage, Re_AcquireNextImage(Re_swapchain))
SIF_INTEGER(SwapchainFormat, Re_SwapchainFormat(Re_swapchain))

SIF_FUNC(SwapchainTexture)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	lua_pushlightuserdata(vm, Re_SwapchainTexture(Re_swapchain, lua_touserdata(vm, 1)));
	return 1;
}

SIF_FUNC(Present)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "");

	Re_Present(Re_swapchain, lua_touserdata(vm, 1), lua_touserdata(vm, 2));
	return 0;
}

// Shader
SIF_FUNC(GetShader)
{
	lua_pushlightuserdata(vm, Re_GetShader(luaL_checkstring(vm, 1)));
	return 1;
}

// Material
SIF_LUSRDATA(MaterialRenderPassDesc, Re_MaterialRenderPassDesc)

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

		// Render Context
		SIF_REG(CmdBindPipeline),
		SIF_REG(CmdPushConstants),
		SIF_REG(CmdBindIndexBuffer),
		SIF_REG(CmdExecuteSecondary),
		SIF_REG(CmdBeginRenderPass),
		SIF_REG(CmdEndRenderPass),
		SIF_REG(CmdSetViewport),
		SIF_REG(CmdSetScissor),
		SIF_REG(CmdDraw),
		SIF_REG(CmdDrawIndexed),
		SIF_REG(CmdDrawIndirect),
		SIF_REG(CmdDrawIndexedIndirect),
		SIF_REG(CmdDispatch),
		SIF_REG(CmdDispatchIndirect),
		SIF_REG(CmdTraceRays),
		SIF_REG(CmdTraceRaysIndirect),
		SIF_REG(CmdBarrier),
		SIF_REG(CmdTransition),
		SIF_REG(CmdCopyBuffer),
		SIF_REG(CmdCopyImage),
		SIF_REG(CmdCopyBufferToImage),
		SIF_REG(CmdCopyImageToBuffer),
		SIF_REG(CmdBlit),
		
		// Pipeline
		SIF_REG(GraphicsPipeline),
		SIF_REG(ComputePipeline),
		SIF_REG(RayTracingPipeline),

		// Texture
		SIF_REG(TextureLayout),
		
		// Buffer
		SIF_REG(CreateBuffer),
		SIF_REG(UpdateBuffer),
		SIF_REG(MapBuffer),
		SIF_REG(UnmapBuffer),
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

		// Material
		SIF_REG(MaterialRenderPassDesc),

		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Re");

	lua_pushinteger(vm, RENDER_COMMANDS_INLINE);
	lua_setglobal(vm, "RENDER_COMMANDS_INLINE");

	lua_pushinteger(vm, SS_ALL);
	lua_setglobal(vm, "SS_ALL");
}

/* NekoEngine
 *
 * l_Render.c
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
