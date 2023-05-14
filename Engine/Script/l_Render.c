#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/RayTracing.h>
#include <Render/Graph/Graph.h>
#include <Script/Interface.h>
#include <Engine/Resource.h>

#include "../Render/Internal.h"

#define SIF_TF_SWAPCHAIN	(TF_INVALID + 1)

enum ScCommandBufferType
{
	SCBT_Graphics = 0,
	SCBT_Compute = 1,
	SCBT_Transfer = 2
};

struct ScCommandBuffer
{
	enum ScCommandBufferType type;
};

SIF_FUNC(__tostring)
{
	struct ScBuffer *sb;
	struct ScTexture *st;
	struct ScCommandBuffer *scb;

	if ((scb = luaL_testudata(vm, 1, SIF_NE_COMMAND_BUFFER))) {
		switch (scb->type) {
		case SCBT_Graphics: lua_pushliteral(vm, "NeCommandBuffer(Graphics)"); break;
		case SCBT_Compute: lua_pushliteral(vm, "NeCommandBuffer(Compute)"); break;
		case SCBT_Transfer: lua_pushliteral(vm, "NeCommandBuffer(Transfer)"); break;
		}
		return 1;
	} else if (luaL_testudata(vm, 1, SIF_NE_RENDER_GRAPH)) {
		lua_pushliteral(vm, "NeRenderGraph");
		return 1;
	} else if (luaL_testudata(vm, 1, SIF_NE_FRAMEBUFFER)) {
		lua_pushliteral(vm, "NeFramebuffer");
		return 1;
	} else if (luaL_testudata(vm, 1, SIF_NE_PIPELINE)) {
		lua_pushliteral(vm, "NePipeline");
		return 1;
	} else if ((sb = luaL_testudata(vm, 1, SIF_NE_BUFFER))) {
		lua_pushfstring(vm, "NeBuffer(%d)", sb->handle);
		return 1;
	} else if ((st = luaL_testudata(vm, 1, SIF_NE_TEXTURE))) {
		lua_pushfstring(vm, "NeTexture(%d)", st->location);
		return 1;
	} else if (luaL_testudata(vm, 1, SIF_NE_SEMAPHORE)) {
		lua_pushliteral(vm, "NeSemaphore");
		return 1;
	} else if (luaL_testudata(vm, 1, SIF_NE_RENDER_PASS_DESC)) {
		lua_pushliteral(vm, "NeRenderPassDesc");
		return 1;
	}

	return 0;
}

// Command Buffer

SIF_FUNC(NewCommandBuffer)
{
	enum ScCommandBufferType type = (float)luaL_checkinteger(vm, 1);

	if (type < SCBT_Graphics || type > SCBT_Transfer)
		luaL_argerror(vm, 1, "must be a valid command buffer type");

	struct ScCommandBuffer *scb = lua_newuserdatauv(vm, sizeof(*scb), 0);
	scb->type = type;
	luaL_setmetatable(vm, SIF_NE_COMMAND_BUFFER);

	switch (scb->type) {
	case SCBT_Graphics: Re_BeginDrawCommandBuffer(lua_touserdata(vm, 2)); break;
	case SCBT_Compute: Re_BeginComputeCommandBuffer(lua_touserdata(vm, 2)); break;
	case SCBT_Transfer: Re_BeginTransferCommandBuffer(lua_touserdata(vm, 2)); break;
	}

	return 1;
}

SIF_FUNC(BindVertexBuffer)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdBindVertexBuffer(buff->handle, luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(BindVertexBuffers)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);

	const uint32_t count = (uint32_t)luaL_checkinteger(vm, 2);
	const uint32_t start = (uint32_t)luaL_optinteger(vm, 5, 0);

	if (!lua_istable(vm, 3))
		luaL_argerror(vm, 3, "Must be a table");

	if (!lua_istable(vm, 4))
		luaL_argerror(vm, 4, "Must be a table");

	NeBufferHandle *handles = Sys_Alloc(sizeof(*handles), count, MH_Transient);
	uint64_t *offsets = Sys_Alloc(sizeof(*offsets), count, MH_Transient);

	for (lua_Integer i = 0; i < count; ++i) {
		lua_rawgeti(vm, 3, i + 1);
		int v = lua_gettop(vm);
		struct ScBuffer *buff = luaL_checkudata(vm, v, SIF_NE_BUFFER);
		handles[i] = buff->handle;
		lua_remove(vm, v);

		lua_rawgeti(vm, 4, i + 1);
		v = lua_gettop(vm);
		offsets[i] = luaL_checkinteger(vm, v);
		lua_remove(vm, v);
	}

	Re_CmdBindVertexBuffers(count, handles, offsets, start);
	return 0;
}

SIF_FUNC(BindIndexBuffer)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdBindIndexBuffer(buff->handle, luaL_checkinteger(vm, 3), (enum NeIndexType)luaL_checkinteger(vm, 4));
	return 0;
}

SIF_FUNC(BindPipeline)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct NeScriptWrapper *sw = luaL_checkudata(vm, 2, SIF_NE_PIPELINE);
	Re_CmdBindPipeline(sw->ptr);
	return 0;
}

SIF_FUNC(PushConstants)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);

	const NeShaderStageFlags stage = (NeShaderStageFlags)luaL_checkinteger(vm, 2);
	if (!lua_istable(vm, 3))
		luaL_argerror(vm, 3, "Must be a table");

	uint8_t *pc = Sys_Alloc(sizeof(*pc), Re_deviceInfo.limits.maxPushConstantsSize, MH_Transient);

	uint32_t offset = 0;
	const uint32_t fields = (uint32_t)lua_rawlen(vm, 3);
	for (uint32_t i = 0; i < fields; ++i) {
		lua_rawgeti(vm, 3, i + 1);
		int v = lua_gettop(vm);

		if (offset >= Re_deviceInfo.limits.maxPushConstantsSize)
			luaL_argerror(vm, v, "Push constant out of range");

		float *f = NULL;
		uint8_t *dst = pc + offset;
		struct ScBuffer *sb;
		struct ScTexture *st;

		if ((f = luaL_testudata(vm, v, SIF_NE_MATRIX))) {
			memcpy(dst, f, sizeof(float) * 16);
			offset += sizeof(float) * 16;
		} else if ((f = luaL_testudata(vm, v, SIF_NE_VEC4))) {
			memcpy(dst, f, sizeof(float) * 4);
			offset += sizeof(float) * 4;
		} else if ((f = luaL_testudata(vm, v, SIF_NE_QUATERNION))) {
			memcpy(dst, f, sizeof(float) * 4);
			offset += sizeof(float) * 4;
		} else if ((f = luaL_testudata(vm, v, SIF_NE_VEC3))) {
			memcpy(dst, f, sizeof(float) * 3);
			offset += sizeof(float) * 3;
		} else if ((sb = luaL_testudata(vm, v, SIF_NE_BUFFER))) {
			*(uint64_t *)dst = sb->address;
			offset += sizeof(uint64_t);
		} else if ((st = luaL_testudata(vm, v, SIF_NE_TEXTURE))) {
			*(uint32_t *)dst = st->location;
			offset += sizeof(uint32_t);
		} else if (lua_isnumber(vm, v)) {
			*(float *)dst = lua_tonumber(vm, v);
			offset += sizeof(float);
		} else if (lua_isinteger(vm, v)) {
			*(uint32_t *)dst = (uint32_t)lua_tointeger(vm, v);
			offset += sizeof(uint32_t);
		} else {
			luaL_argerror(vm, v, "Unknown argument type");
		}

		lua_remove(vm, v);
	}

	Re_CmdPushConstants(stage, offset, pc);
	return 0;
}

SIF_FUNC(BeginRenderPass)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "");

	if (!lua_islightuserdata(vm, 3))
		luaL_argerror(vm, 3, "");

	Re_CmdBeginRenderPass(lua_touserdata(vm, 2), lua_touserdata(vm, 3), RENDER_COMMANDS_INLINE);
	return 0;
}

SIF_FUNC(EndRenderPass)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdEndRenderPass();
	return 0;
}

SIF_FUNC(Viewport)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdSetViewport(luaL_checknumber(vm, 2), luaL_checknumber(vm, 3),
						luaL_checknumber(vm, 4), luaL_checknumber(vm, 5),
						luaL_checknumber(vm, 6), luaL_checknumber(vm, 7));
	return 0;
}

SIF_FUNC(Scissor)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdSetScissor((uint32_t)luaL_checkinteger(vm, 2), (uint32_t)luaL_checkinteger(vm, 3),
						(uint32_t)luaL_checkinteger(vm, 4), (uint32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(LineWidth)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdSetLineWidth(luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(Draw)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdDraw((int32_t)luaL_checkinteger(vm, 2), (int32_t)luaL_checkinteger(vm, 3),
				(int32_t)luaL_checkinteger(vm, 4), (int32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(DrawIndexed)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdDrawIndexed((int32_t)luaL_checkinteger(vm, 2), (int32_t)luaL_checkinteger(vm, 3),
						(int32_t)luaL_checkinteger(vm, 4), (int32_t)luaL_checkinteger(vm, 5),
						(int32_t)luaL_checkinteger(vm, 6));
	return 0;
}

SIF_FUNC(DrawIndirect)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdDrawIndirect(buff->buff, (uint64_t)luaL_checkinteger(vm, 3),
						(uint32_t)luaL_checkinteger(vm, 4), (uint32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(DrawIndexedIndirect)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdDrawIndexedIndirect(buff->buff, (uint64_t)luaL_checkinteger(vm, 3),
								(uint32_t)luaL_checkinteger(vm, 4), (uint32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(DrawMeshTasks)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdDrawMeshTasks((int32_t)luaL_checkinteger(vm, 2), (int32_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(DrawMeshTasksIndirect)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdDrawIndirect(buff->buff, (uint64_t)luaL_checkinteger(vm, 3),
						(uint32_t)luaL_checkinteger(vm, 4), (uint32_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(DrawMeshTasksIndirectCount)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	struct ScBuffer *countBuff = luaL_checkudata(vm, 4, SIF_NE_BUFFER);
	Re_CmdDrawMeshTasksIndirectCount(buff->buff, (uint64_t)luaL_checkinteger(vm, 3), countBuff->buff,
									(uint64_t)luaL_checkinteger(vm, 5), (uint32_t)luaL_checkinteger(vm, 6),
									(uint32_t)luaL_checkinteger(vm, 7));
	return 0;
}

SIF_FUNC(Dispatch)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdDispatch((uint32_t)luaL_checkinteger(vm, 1), (uint32_t)luaL_checkinteger(vm, 2), (uint32_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(DispatchIndirect)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdDispatchIndirect(buff->buff, (uint64_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(TraceRays)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	Re_CmdTraceRays((uint32_t)luaL_checkinteger(vm, 1), (uint32_t)luaL_checkinteger(vm, 2), (uint32_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(TraceRaysIndirect)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	Re_CmdTraceRaysIndirect(buff->buff, (uint64_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(Blit)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScTexture *src = luaL_checkudata(vm, 2, SIF_NE_TEXTURE);
	struct ScTexture *dst = luaL_checkudata(vm, 3, SIF_NE_TEXTURE);

	if (lua_istable(vm, 4))
		luaL_argerror(vm, 4, "Must be a table");

	struct NeBlitRegion *regions = NULL;
	const uint32_t regionCount = (uint32_t)lua_rawlen(vm, 4);
	if (regionCount) {
		regions = Sys_Alloc(sizeof(*regions), regionCount, MH_Transient);
		for (lua_Integer i = 0; i < regionCount; ++i) {
			lua_rawgeti(vm, 4, i + 1);
			int v = lua_gettop(vm);

			int t = SIF_GETFIELD(1, "src");
			regions[i].srcSize.width = SIF_INTFIELD(t, "width");
			regions[i].srcSize.height = SIF_INTFIELD(t, "height");

			regions[i].srcOffset.x = SIF_OPTINTFIELD(t, "x", 0);
			regions[i].srcOffset.y = SIF_OPTINTFIELD(t, "y", 0);
			regions[i].srcOffset.z = SIF_OPTINTFIELD(t, "z", 0);
			regions[i].srcSize.depth = SIF_OPTINTFIELD(t, "depth", 1);
			regions[i].srcSubresource.aspect = SIF_OPTINTFIELD(t, "aspect", IA_COLOR);
			regions[i].srcSubresource.mipLevel = SIF_OPTINTFIELD(t, "mipLevel", 0);
			regions[i].srcSubresource.baseArrayLayer = SIF_OPTINTFIELD(t, "baseArrayLayer", 0);
			regions[i].srcSubresource.layerCount = SIF_OPTINTFIELD(t, "layerCount", 1);
			regions[i].srcSubresource.levelCount = SIF_OPTINTFIELD(t, "levelCount", 1);
			SIF_POPFIELD(t);

			t = SIF_GETFIELD(1, "dst");
			regions[i].dstSize.width = SIF_INTFIELD(t, "width");
			regions[i].dstSize.height = SIF_INTFIELD(t, "height");

			regions[i].dstOffset.x = SIF_OPTINTFIELD(t, "x", 0);
			regions[i].dstOffset.y = SIF_OPTINTFIELD(t, "y", 0);
			regions[i].dstOffset.z = SIF_OPTINTFIELD(t, "z", 0);
			regions[i].dstSize.depth = SIF_OPTINTFIELD(t, "depth", 1);
			regions[i].dstSubresource.aspect = SIF_OPTINTFIELD(t, "aspect", IA_COLOR);
			regions[i].dstSubresource.mipLevel = SIF_OPTINTFIELD(t, "mipLevel", 0);
			regions[i].dstSubresource.baseArrayLayer = SIF_OPTINTFIELD(t, "baseArrayLayer", 0);
			regions[i].dstSubresource.layerCount = SIF_OPTINTFIELD(t, "layerCount", 1);
			regions[i].dstSubresource.levelCount = SIF_OPTINTFIELD(t, "levelCount", 1);
			SIF_POPFIELD(t);

			lua_remove(vm, v);
		}
	}

	Re_CmdBlit(src->tex, dst->tex, regions, regionCount, (enum NeImageFilter)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(Barrier)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);

	if (!lua_istable(vm, 2))
		luaL_argerror(vm, 2, "Must be a table");

	enum NePipelineDependency dep = SIF_OPTINTFIELD(1, "pipelineDependency", RE_PD_BY_REGION);

	int t = SIF_GETFIELD(1, "memory");
	struct NeMemoryBarrier *memBarriers = NULL;
	const uint32_t memBarrierCount = (uint32_t)lua_rawlen(vm, t);
	if (memBarrierCount) {
		memBarriers = Sys_Alloc(sizeof(*memBarriers), memBarrierCount, MH_Transient);
		for (lua_Integer i = 0; i < memBarrierCount; ++i) {
			lua_rawgeti(vm, t, i + 1);
			int v = lua_gettop(vm);

			memBarriers[i].srcStage = SIF_U64FIELD(v, "srcStage");
			memBarriers[i].dstStage = SIF_U64FIELD(v, "dstStage");
			memBarriers[i].srcAccess = SIF_U64FIELD(v, "srcAccess");
			memBarriers[i].dstAccess = SIF_U64FIELD(v, "dstAccess");

			lua_remove(vm, v);
		}
	}
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(1, "buffer");
	struct NeBufferBarrier *buffBarriers = NULL;
	const uint32_t buffBarrierCount = (uint32_t)lua_rawlen(vm, t);
	if (buffBarrierCount) {
		buffBarriers = Sys_Alloc(sizeof(*buffBarriers), buffBarrierCount, MH_Transient);
		for (lua_Integer i = 0; i < buffBarrierCount; ++i) {
			lua_rawgeti(vm, t, i + 1);
			int v = lua_gettop(vm);

			struct ScBuffer *buff = luaL_checkudata(vm, v, SIF_NE_BUFFER);
			buffBarriers[i].buffer = buff->buff;

			memBarriers[i].srcStage = SIF_OPTU64FIELD(v, "srcStage", RE_PS_ALL_COMMANDS);
			memBarriers[i].dstStage = SIF_OPTU64FIELD(v, "dstStage", RE_PS_ALL_COMMANDS);
			memBarriers[i].srcAccess = SIF_OPTU64FIELD(v, "srcAccess", RE_PA_MEMORY_WRITE);
			memBarriers[i].dstAccess = SIF_OPTU64FIELD(v, "dstAccess", RE_PA_MEMORY_READ);

			buffBarriers[i].offset = SIF_OPTU64FIELD(v, "offset", 0);
			buffBarriers[i].size = SIF_OPTU64FIELD(v, "size", RE_WHOLE_SIZE);
			buffBarriers[i].srcQueue = SIF_OPTINTFIELD(v, "srcQueue", RE_QUEUE_GRAPHICS);
			buffBarriers[i].dstQueue = SIF_OPTINTFIELD(v, "dstQueue", RE_QUEUE_GRAPHICS);

			lua_remove(vm, v);
		}
	}
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(1, "image");
	struct NeImageBarrier *imgBarriers = NULL;
	const uint32_t imgBarrierCount = (uint32_t)lua_rawlen(vm, t);
	if (imgBarrierCount) {
		imgBarriers = Sys_Alloc(sizeof(*imgBarriers), imgBarrierCount, MH_Transient);
		for (lua_Integer i = 0; i < imgBarrierCount; ++i) {
			lua_rawgeti(vm, t, i + 1);
			int v = lua_gettop(vm);

			imgBarriers[i].oldLayout = SIF_INTFIELD(v, "oldLayout");
			imgBarriers[i].newLayout = SIF_INTFIELD(v, "newLayout");

			struct ScTexture *tex = luaL_checkudata(vm, v, SIF_NE_TEXTURE);
			imgBarriers[i].texture = tex->tex;

			memBarriers[i].srcStage = SIF_OPTU64FIELD(v, "srcStage", RE_PS_ALL_COMMANDS);
			memBarriers[i].dstStage = SIF_OPTU64FIELD(v, "dstStage", RE_PS_ALL_COMMANDS);
			memBarriers[i].srcAccess = SIF_OPTU64FIELD(v, "srcAccess", RE_PA_MEMORY_WRITE);
			memBarriers[i].dstAccess = SIF_OPTU64FIELD(v, "dstAccess", RE_PA_MEMORY_READ);

			imgBarriers[i].subresource.aspect = SIF_OPTINTFIELD(v, "aspect", IA_COLOR);
			imgBarriers[i].subresource.baseArrayLayer = SIF_OPTINTFIELD(v, "baseArrayLayer", 0);
			imgBarriers[i].subresource.layerCount = SIF_OPTINTFIELD(v, "layerCount", 1);
			imgBarriers[i].subresource.mipLevel = SIF_OPTINTFIELD(v, "mipLevel", 0);
			imgBarriers[i].subresource.levelCount = SIF_OPTINTFIELD(v, "levelCount", 1);

			imgBarriers[i].srcQueue = SIF_OPTINTFIELD(v, "srcQueue", RE_QUEUE_GRAPHICS);
			imgBarriers[i].dstQueue = SIF_OPTINTFIELD(v, "dstQueue", RE_QUEUE_GRAPHICS);

			lua_remove(vm, v);
		}
	}
	SIF_POPFIELD(t);

	Re_CmdBarrier(dep, memBarrierCount, memBarriers, buffBarrierCount, buffBarriers, imgBarrierCount, imgBarriers);
	return 0;
}

SIF_FUNC(UpdateBuffer)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);

	if (!lua_islightuserdata(vm, 4))
		luaL_argerror(vm, 4, "");

	Re_CmdUpdateBuffer(buff->handle, (uint64_t)luaL_checkinteger(vm, 3),
						lua_touserdata(vm, 4), (uint64_t)luaL_checkinteger(vm, 5));
	return 0;
}

SIF_FUNC(CopyBuffer)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *src = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	struct ScBuffer *dst = luaL_checkudata(vm, 4, SIF_NE_BUFFER);
	Re_CmdCopyBuffer(src->handle, (uint64_t)luaL_checkinteger(vm, 3),
						dst->handle, (uint64_t)luaL_checkinteger(vm, 5),
						(uint64_t)luaL_optinteger(vm, 6, RE_WHOLE_SIZE));
	return 0;
}

SIF_FUNC(CopyBufferToTexture)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 2, SIF_NE_BUFFER);
	struct ScTexture *tex = luaL_checkudata(vm, 2, SIF_NE_TEXTURE);

	if (!lua_istable(vm, 4))
		luaL_argerror(vm, 4, "Must be a table");

	struct NeBufferImageCopy *bic = Sys_Alloc(sizeof(*bic), 1, MH_Transient);
	bic->bufferOffset = SIF_U64FIELD(4, "bufferOffset");
	bic->bytesPerRow = SIF_INTFIELD(4, "bytesPerRow");
	bic->rowLength = SIF_INTFIELD(4, "rowLength");
	bic->imageHeight = SIF_INTFIELD(4, "imageHeight");

	bic->imageOffset.x = SIF_OPTINTFIELD(4, "x", 0);
	bic->imageOffset.y = SIF_OPTINTFIELD(4, "y", 0);
	bic->imageOffset.z = SIF_OPTINTFIELD(4, "z", 0);

	bic->imageSize.width = SIF_INTFIELD(4, "x");
	bic->imageSize.height = SIF_INTFIELD(4, "y");
	bic->imageSize.depth = SIF_OPTINTFIELD(4, "z", 1);

	bic->subresource.aspect = SIF_OPTINTFIELD(4, "imageAspect", IA_COLOR);
	bic->subresource.mipLevel = SIF_OPTINTFIELD(4, "mipLevel", 0);
	bic->subresource.baseArrayLayer = SIF_OPTINTFIELD(4, "baseArrayLayer", 0);
	bic->subresource.layerCount = SIF_OPTINTFIELD(4, "layerCount", 1);
	bic->subresource.levelCount = SIF_OPTINTFIELD(4, "levelCount", 1);

	Re_CmdCopyBufferToTexture(buff->handle, tex->tex, bic);
	return 0;
}

SIF_FUNC(CopyTextureToBuffer)
{
	luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	struct ScBuffer *buff = luaL_checkudata(vm, 3, SIF_NE_BUFFER);
	struct ScTexture *tex = luaL_checkudata(vm, 2, SIF_NE_TEXTURE);

	if (!lua_istable(vm, 4))
		luaL_argerror(vm, 4, "Must be a table");

	struct NeBufferImageCopy *bic = Sys_Alloc(sizeof(*bic), 1, MH_Transient);
	bic->bufferOffset = SIF_U64FIELD(4, "bufferOffset");
	bic->bytesPerRow = SIF_INTFIELD(4, "bytesPerRow");
	bic->rowLength = SIF_INTFIELD(4, "rowLength");
	bic->imageHeight = SIF_INTFIELD(4, "imageHeight");

	bic->imageOffset.x = SIF_OPTINTFIELD(4, "x", 0);
	bic->imageOffset.y = SIF_OPTINTFIELD(4, "y", 0);
	bic->imageOffset.z = SIF_OPTINTFIELD(4, "z", 0);

	bic->imageSize.width = SIF_INTFIELD(4, "x");
	bic->imageSize.height = SIF_INTFIELD(4, "y");
	bic->imageSize.depth = SIF_OPTINTFIELD(4, "z", 1);

	bic->subresource.aspect = SIF_OPTINTFIELD(4, "imageAspect", IA_COLOR);
	bic->subresource.mipLevel = SIF_OPTINTFIELD(4, "mipLevel", 0);
	bic->subresource.baseArrayLayer = SIF_OPTINTFIELD(4, "baseArrayLayer", 0);
	bic->subresource.layerCount = SIF_OPTINTFIELD(4, "layerCount", 1);
	bic->subresource.levelCount = SIF_OPTINTFIELD(4, "levelCount", 1);

	Re_CmdCopyTextureToBuffer(tex->tex, buff->handle, bic);
	return 0;
}

SIF_FUNC(Queue)
{
	struct ScCommandBuffer *scb = luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	switch (scb->type) {
		case SCBT_Graphics: Re_QueueGraphics(Re_EndCommandBuffer(), lua_touserdata(vm, 2)); break;
		case SCBT_Compute: Re_QueueCompute(Re_EndCommandBuffer(), lua_touserdata(vm, 2)); break;
		case SCBT_Transfer: Re_QueueTransfer(Re_EndCommandBuffer(), lua_touserdata(vm, 2)); break;
	}
	return 0;
}

SIF_FUNC(Execute)
{
	struct ScCommandBuffer *scb = luaL_checkudata(vm, 1, SIF_NE_COMMAND_BUFFER);
	switch (scb->type) {
		case SCBT_Graphics: Re_ExecuteGraphics(Re_EndCommandBuffer()); break;
		case SCBT_Compute: Re_ExecuteCompute(Re_EndCommandBuffer()); break;
		case SCBT_Transfer: Re_ExecuteTransfer(Re_EndCommandBuffer()); break;
	}
	return 0;
}

static void
RegisterCommandBuffer(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(BindVertexBuffer),
		SIF_REG(BindVertexBuffers),
		SIF_REG(BindIndexBuffer),
		SIF_REG(BindPipeline),
		SIF_REG(PushConstants),
		SIF_REG(BeginRenderPass),
		SIF_REG(EndRenderPass),
		SIF_REG(Viewport),
		SIF_REG(Scissor),
		SIF_REG(LineWidth),
		SIF_REG(Draw),
		SIF_REG(DrawIndexed),
		SIF_REG(DrawIndirect),
		SIF_REG(DrawIndexedIndirect),
		SIF_REG(DrawMeshTasks),
		SIF_REG(DrawMeshTasksIndirect),
		SIF_REG(DrawMeshTasksIndirectCount),
		SIF_REG(Dispatch),
		SIF_REG(DispatchIndirect),
		SIF_REG(TraceRays),
		SIF_REG(TraceRaysIndirect),
		SIF_REG(Blit),
		SIF_REG(Barrier),
		SIF_REG(UpdateBuffer),
		SIF_REG(CopyBuffer),
		SIF_REG(CopyBufferToTexture),
		SIF_REG(CopyTextureToBuffer),
		SIF_REG(Queue),
		SIF_REG(Execute),
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_COMMAND_BUFFER, Sif_NewCommandBuffer);
	luaL_newmetatable(vm, SIF_NE_COMMAND_BUFFER);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
}

// Graph
SIF_FUNC(AddTexture)
{
	struct NeRenderGraph *rg = luaL_checkudata(vm, 1, SIF_NE_RENDER_GRAPH);
	struct NeTextureDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);
	desc->width = SIF_INTFIELD(3, "width");
	desc->height = SIF_INTFIELD(3, "height");
	desc->type = SIF_INTFIELD(3, "type");
	desc->usage = SIF_INTFIELD(3, "usage");
	desc->arrayLayers = SIF_OPTINTFIELD(3, "arrayLayers", 1);
	desc->mipLevels = SIF_OPTINTFIELD(3, "mipLevels", 1);
	desc->memoryType = SIF_OPTINTFIELD(3, "memoryType", MT_GPU_LOCAL);
	desc->gpuOptimalTiling = SIF_BOOLFIELD(3, "gpuOptimalTiling");
	desc->name = SIF_STRINGFIELD(1, "width");

	int f = SIF_GETFIELD(1, "format");
	if (lua_isinteger(vm, f)) {
		desc->format = (enum NeTextureFormat)lua_tointeger(vm, f);
	} else {
		size_t len;
		const char *str = luaL_checklstring(vm, f, &len);

		if (!strncmp("swapchain", str, len))
			desc->format = Re_SwapchainFormat(Re_swapchain);
		else
			luaL_argerror(vm, f, "format must be a TextureFormat or \"swapchain\"");
	}
	SIF_POPFIELD(f);

	lua_pushboolean(vm, Re_AddGraphTexture(luaL_checkstring(vm, 2), desc, (uint16_t)luaL_checkinteger(vm, 4), &rg->resources));
	return 1;
}

SIF_FUNC(AddBuffer)
{
	struct NeRenderGraph *rg = luaL_checkudata(vm, 1, SIF_NE_RENDER_GRAPH);
	struct NeBufferDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);
	desc->size = SIF_U64FIELD(3, "size");
	desc->usage = SIF_INTFIELD(3, "usage");
	desc->memoryType = SIF_OPTINTFIELD(3, "memoryType", MT_GPU_LOCAL);
	desc->name = SIF_STRINGFIELD(1, "width");

	lua_pushboolean(vm, Re_AddGraphBuffer(luaL_checkstring(vm, 2), desc, &rg->resources));
	return 1;
}

SIF_FUNC(AddData)
{
	struct NeRenderGraph *rg = luaL_checkudata(vm, 1, SIF_NE_RENDER_GRAPH);
	lua_pushboolean(vm, Re_AddGraphBuffer(luaL_checkstring(vm, 2), lua_touserdata(vm, 3), &rg->resources));
	return 1;
}

SIF_FUNC(Texture)
{
	struct NeRenderGraph *rg = luaL_checkudata(vm, 1, SIF_NE_RENDER_GRAPH);
	struct ScTexture *tex = lua_newuserdatauv(vm, sizeof(*tex), 0);

	struct NeTextureDesc *desc = NULL;
	tex->tex = Re_GraphTexture(Rt_HashString(luaL_checkstring(vm, 2)), &rg->resources, &tex->location, &desc);
	memcpy(&tex->desc, desc, sizeof(tex->desc));

	return 1;
}

SIF_FUNC(Buffer)
{
	struct NeRenderGraph *rg = luaL_checkudata(vm, 1, SIF_NE_RENDER_GRAPH);
	struct ScBuffer *buff = lua_newuserdatauv(vm, sizeof(*buff), 0);
	buff->address = Re_GraphBuffer(Rt_HashString(luaL_checkstring(vm, 2)), &rg->resources, &buff->buff);
	return 1;
}

SIF_FUNC(Data)
{
	struct NeRenderGraph *rg = luaL_checkudata(vm, 1, SIF_NE_RENDER_GRAPH);
	// convert to table
	lua_pushlightuserdata(vm, Re_GraphData(Rt_HashString(luaL_checkstring(vm, 2)), &rg->resources));
	return 1;
}

static void
RegisterRenderGraph(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Texture),
		SIF_REG(Buffer),
		SIF_REG(Data),
		SIF_REG(AddTexture),
		SIF_REG(AddBuffer),
		SIF_REG(AddData),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, SIF_NE_RENDER_GRAPH);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
}

// Framebuffer
SIF_FUNC(NewFramebuffer)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeFramebufferDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);

	desc->height = SIF_INTFIELD(1, "height");
	desc->width = SIF_INTFIELD(1, "width");
	struct NeScriptWrapper *sw = SIF_USRDATAFIELD(1, "renderPassDesc", SIF_NE_RENDER_PASS_DESC);
	desc->renderPassDesc = sw->ptr;

	desc->layers = SIF_OPTINTFIELD(1, "layers", 1);
	desc->name = SIF_OPTSTRINGFIELD(1, "name", NULL);

	int t = SIF_GETFIELD(1, "attachments");
	desc->attachmentCount =  (uint32_t)lua_rawlen(vm, t);
	desc->attachments = desc->attachmentCount ? Sys_Alloc(sizeof(*desc->attachments), desc->attachmentCount, MH_Transient) : NULL;
	for (lua_Integer i = 0; i < desc->attachmentCount; ++i) {
		lua_rawgeti(vm, t, i + 1);
		int v = lua_gettop(vm);

		int f = SIF_GETFIELD(v, "format");
		if (lua_isinteger(vm, f)) {
			desc->attachments[i].format = (enum NeTextureFormat)lua_tointeger(vm, f);
		} else {
			size_t len;
			const char *str = luaL_checklstring(vm, f, &len);

			if (!strncmp("swapchain", str, len))
				Re_SwapchainDesc(Re_swapchain, &desc->attachments[i]);
			else
				luaL_argerror(vm, f, "format must be a TextureFormat or \"swapchain\"");
		}
		SIF_POPFIELD(f);

		desc->attachments[i].usage = SIF_OPTINTFIELD(v, "usage", desc->attachments[i].usage);

		lua_remove(vm, v);
	}
	SIF_POPFIELD(t);

	sw = lua_newuserdatauv(vm, sizeof(*sw), 0);
	sw->ptr = Re_CreateFramebuffer(desc);
	luaL_setmetatable(vm, SIF_NE_FRAMEBUFFER);
	return 1;
}

SIF_FUNC(Attachment)
{
	SIF_CHECKCOMPONENT(1, fb, SIF_NE_FRAMEBUFFER, struct NeFramebuffer *);
	struct ScTexture *tex = luaL_checkudata(vm, 3, SIF_NE_TEXTURE);
	Re_SetAttachment(fb, (uint32_t)luaL_checkinteger(vm, 2), tex->tex);
	return 0;
}

SIF_FUNC(DestroyFramebuffer)
{
	SIF_CHECKCOMPONENT(1, fb, SIF_NE_FRAMEBUFFER, struct NeFramebuffer *);
	Re_Destroy(fb);
	return 0;
}

static void
RegisterFramebuffer(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Attachment),
		{ "Destroy", Sif_DestroyFramebuffer },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_FRAMEBUFFER, Sif_NewFramebuffer);
	luaL_newmetatable(vm, SIF_NE_FRAMEBUFFER);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
}

// Pipeline
SIF_FUNC(GraphicsPipeline)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeGraphicsPipelineDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);

	desc->flags = 0;

	switch (SIF_OPTINTFIELD(1, "primitiveType", PT_TRIANGLES)) {
	case PT_TRIANGLES: desc->flags |= RE_TOPOLOGY_POINTS; break;
	case PT_LINES: desc->flags |= RE_TOPOLOGY_LINES; break;
	case PT_POINTS: desc->flags |= RE_TOPOLOGY_POINTS; break;
	}

	desc->flags |= SIF_OPTU64FIELD(1, "fillMode", RE_POLYGON_FILL);
	desc->flags |= SIF_OPTU64FIELD(1, "cullMode", RE_CULL_BACK);
	desc->flags |= SIF_OPTU64FIELD(1, "frontFace", RE_FRONT_FACE_CCW);

	switch (SIF_OPTINTFIELD(1, "depthOp", CO_GREATER_EQUAL)) {
	case CO_NEVER: desc->flags |= RE_DEPTH_OP_NEVER; break;
	case CO_LESS: desc->flags |= RE_DEPTH_OP_LESS; break;
	case CO_EQUAL: desc->flags |= RE_DEPTH_OP_EQUAL; break;
	case CO_LESS_EQUAL: desc->flags |= RE_DEPTH_OP_LESS_EQUAL; break;
	case CO_GREATER: desc->flags |= RE_DEPTH_OP_GREATER; break;
	case CO_NOT_EQUAL: desc->flags |= RE_DEPTH_OP_NOT_EQUAL; break;
	case CO_GREATER_EQUAL: desc->flags |= RE_DEPTH_OP_GREATER_EQUAL; break;
	case CO_ALWAYS: desc->flags |= RE_DEPTH_OP_ALWAYS; break;
	}

	if (SIF_OPTBOOLFIELD(1, "discard", false))
		desc->flags |= RE_DISCARD;

	if (SIF_OPTBOOLFIELD(1, "depthTest", false))
		desc->flags |= RE_DEPTH_TEST;

	if (SIF_OPTBOOLFIELD(1, "depthWrite", false))
		desc->flags |= RE_DEPTH_WRITE;

	if (SIF_OPTBOOLFIELD(1, "depthBias", false))
		desc->flags |= RE_DEPTH_BIAS;

	if (SIF_OPTBOOLFIELD(1, "depthClamp", false))
		desc->flags |= RE_DEPTH_CLAMP;

	if (SIF_OPTBOOLFIELD(1, "depthBounds", false))
		desc->flags |= RE_DEPTH_BOUNDS;

	if (SIF_OPTBOOLFIELD(1, "sampleShading", false))
		desc->flags |= RE_SAMPLE_SHADING;

	if (SIF_OPTBOOLFIELD(1, "alphaToCoverage", false))
		desc->flags |= RE_ALPHA_TO_COVERAGE;

	if (SIF_OPTBOOLFIELD(1, "alphaToOne", false))
		desc->flags |= RE_ALPHA_TO_ONE;

	if (SIF_OPTBOOLFIELD(1, "multisample", false))
		desc->flags |= RE_MULTISAMPLE;

	switch (SIF_OPTINTFIELD(1, "samples", 0)) {
	case 2: desc->flags |= RE_MS_2_SAMPLES; break;
	case 4: desc->flags |= RE_MS_4_SAMPLES; break;
	case 8: desc->flags |= RE_MS_8_SAMPLES; break;
	case 16: desc->flags |= RE_MS_16_SAMPLES; break;
	}

	struct NeShader *s = Re_GetShader(SIF_STRINGFIELD(1, "shader"));
	if (!s)
		luaL_argerror(vm, 1, "The specified shader does not exist");

	desc->stageInfo = &s->opaqueStages;

	struct NeScriptWrapper *sw = NULL;
	int t = SIF_GETFIELD(1, "renderPassDesc");
	if ((sw = luaL_testudata(vm, t, SIF_NE_RENDER_PASS_DESC))) {
		desc->renderPassDesc = sw->ptr;
	} else {
		size_t len;
		const char *str = luaL_checklstring(vm, t, &len);

		if (!strncmp("material", str, len))
			desc->renderPassDesc = Re_MaterialRenderPassDesc;
		else
			luaL_argerror(vm, t, "renderPassDesc must be a valid render pass desc or \"material\"");
	}
	SIF_POPFIELD(t);

	desc->pushConstantSize = SIF_INTFIELD(1, "pushConstantSize");

	t = SIF_GETFIELD(1, "vertexDesc");
	if (lua_istable(vm, t)) {
		int f = SIF_GETFIELD(t, "attributes");
		desc->vertexDesc.attributeCount = (uint32_t)lua_rawlen(vm, f);
		if (desc->vertexDesc.attributeCount) {
			desc->vertexDesc.attributes = Sys_Alloc(sizeof(*desc->vertexDesc.attributes), desc->vertexDesc.attributeCount, MH_Transient);
			for (uint32_t i = 0; i < desc->vertexDesc.attributeCount; ++i) {
				lua_rawgeti(vm, f, i + 1);
				int v = lua_gettop(vm);

				desc->vertexDesc.attributes[i].location = SIF_INTFIELD(v, "location");
				desc->vertexDesc.attributes[i].binding = SIF_INTFIELD(v, "binding");
				desc->vertexDesc.attributes[i].format = SIF_INTFIELD(v, "format");
				desc->vertexDesc.attributes[i].offset = SIF_INTFIELD(v, "offset");
				desc->vertexDesc.attributes[i].semantic = SIF_INTFIELD(v, "semantic");

				lua_remove(vm, v);
			}
		}
		SIF_POPFIELD(f);

		f = SIF_GETFIELD(t, "bindings");
		desc->vertexDesc.bindingCount = (uint32_t)lua_rawlen(vm, f);
		if (desc->vertexDesc.bindingCount) {
			desc->vertexDesc.bindings = Sys_Alloc(sizeof(*desc->vertexDesc.bindings), desc->vertexDesc.bindingCount, MH_Transient);
			for (uint32_t i = 0; i < desc->vertexDesc.bindingCount; ++i) {
				lua_rawgeti(vm, f, i + 1);
				int v = lua_gettop(vm);

				desc->vertexDesc.bindings[i].binding = SIF_INTFIELD(v, "binding");
				desc->vertexDesc.bindings[i].stride = SIF_INTFIELD(v, "stride");
				desc->vertexDesc.bindings[i].inputRate = SIF_OPTINTFIELD(v, "inputRate", VIR_VERTEX);

				lua_remove(vm, v);
			}
		}
		SIF_POPFIELD(f);
	}
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(1, "blendAttachments");
	desc->attachmentCount = (uint32_t)lua_rawlen(vm, t);
	struct NeBlendAttachmentDesc *atDesc = desc->attachmentCount ? Sys_Alloc(sizeof(*desc->attachments), desc->attachmentCount, MH_Transient) : NULL;
	desc->attachments = atDesc;

	for (lua_Integer i = 0; i < desc->attachmentCount; ++i) {
		lua_rawgeti(vm, t, i + 1);
		int v = lua_gettop(vm);

		atDesc[i].enableBlend = SIF_BOOLFIELD(v, "enableBlend");
		atDesc[i].srcColor = SIF_OPTINTFIELD(v, "srcColor", RE_BF_SRC_ALPHA);
		atDesc[i].dstColor = SIF_OPTINTFIELD(v, "dstColor", RE_BF_ONE_MINUS_SRC_ALPHA);
		atDesc[i].colorOp = SIF_OPTINTFIELD(v, "colorOp", RE_BOP_ADD);
		atDesc[i].srcAlpha = SIF_OPTINTFIELD(v, "srcAlpha", RE_BF_ONE);
		atDesc[i].dstAlpha = SIF_OPTINTFIELD(v, "dstAlpha", RE_BF_ZERO);
		atDesc[i].alphaOp = SIF_OPTINTFIELD(v, "alphaOp", RE_BOP_ADD);
		atDesc[i].writeMask = SIF_OPTINTFIELD(v, "writeMask", RE_WRITE_MASK_RGBA);

		lua_remove(vm, v);
	}
	SIF_POPFIELD(t);

	desc->name = SIF_OPTSTRINGFIELD(1, "name", NULL);

	sw = lua_newuserdatauv(vm, sizeof(*sw), 0);
	sw->ptr = Re_GraphicsPipeline(desc);
	luaL_setmetatable(vm, SIF_NE_PIPELINE);
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
	desc->name = SIF_OPTSTRINGFIELD(1, "name", NULL);

	struct NeScriptWrapper *sw = lua_newuserdatauv(vm, sizeof(*sw), 0);
	sw->ptr = Re_ComputePipeline(desc);
	luaL_setmetatable(vm, SIF_NE_PIPELINE);
	return 1;
}

SIF_FUNC(RayTracingPipeline)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeRayTracingPipelineDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);
	desc->stageInfo = SIF_LUSRDATAFIELD(1, "stageInfo");
	desc->sbt = SIF_LUSRDATAFIELD(1, "shaderBindingTable");
	desc->maxDepth = SIF_INTFIELD(1, "maxDepth");
	desc->name = SIF_OPTSTRINGFIELD(1, "name", NULL);

	struct NeScriptWrapper *sw = lua_newuserdatauv(vm, sizeof(*sw), 0);
	sw->ptr = Re_RayTracingPipeline(desc);
	luaL_setmetatable(vm, SIF_NE_PIPELINE);
	return 1;
}

// Render Pass
SIF_FUNC(NewRenderPassDesc)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	int t = SIF_GETFIELD(1, "output");
	const uint32_t outCount = lua_istable(vm, t) ? (uint32_t)lua_rawlen(vm, t) : 0;
	struct NeAttachmentDesc *outDesc = NULL;
	if (lua_istable(vm, t)) {
		outDesc = Sys_Alloc(sizeof(*outDesc), outCount, MH_Transient);
		for (lua_Integer i = 0; i < outCount; ++i) {
			lua_rawgeti(vm, t, i + 1);
			int v = lua_gettop(vm);

			int f = SIF_GETFIELD(v, "format");
			if (lua_isinteger(vm, f)) {
				outDesc[i].format = (enum NeTextureFormat)lua_tointeger(vm, f);
			} else {
				size_t len;
				const char *str = luaL_checklstring(vm, f, &len);

				if (!strncmp("swapchain", str, len))
					outDesc[i].format = Re_SwapchainFormat(Re_swapchain);
				else
					luaL_argerror(vm, f, "format must be a TextureFormat or \"swapchain\"");
			}
			SIF_POPFIELD(f);

			outDesc[i].layout = SIF_OPTINTFIELD(v, "layout", TL_COLOR_ATTACHMENT);
			outDesc[i].initialLayout = SIF_OPTINTFIELD(v, "initialLayout", TL_COLOR_ATTACHMENT);
			outDesc[i].finalLayout = SIF_OPTINTFIELD(v, "finalLayout", TL_COLOR_ATTACHMENT);
			outDesc[i].samples = SIF_OPTINTFIELD(v, "samples", 1);
			outDesc[i].loadOp = SIF_OPTINTFIELD(v, "load", ATL_CLEAR);
			outDesc[i].storeOp = SIF_OPTINTFIELD(v, "store", ATS_STORE);
			outDesc[i].mayAlias = SIF_OPTBOOLFIELD(v, "mayAlias", false);

			// TODO: clearColor

			lua_remove(vm, v);
		}
	}
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(1, "depth");
	struct NeAttachmentDesc *depthDesc = NULL;
	if (lua_istable(vm, t)) {
		depthDesc = Sys_Alloc(sizeof(*depthDesc), 1, MH_Transient);
		depthDesc->format = SIF_INTFIELD(t, "format");

		depthDesc->layout = SIF_OPTINTFIELD(t, "layout", TL_DEPTH_ATTACHMENT);
		depthDesc->initialLayout = SIF_OPTINTFIELD(t, "initialLayout", TL_DEPTH_ATTACHMENT);
		depthDesc->finalLayout = SIF_OPTINTFIELD(t, "finalLayout", TL_DEPTH_ATTACHMENT);
		depthDesc->samples = SIF_OPTINTFIELD(t, "samples", 1);
		depthDesc->loadOp = SIF_OPTINTFIELD(t, "load", ATL_CLEAR);
		depthDesc->storeOp = SIF_OPTINTFIELD(t, "store", ATS_STORE);
		depthDesc->mayAlias = SIF_OPTBOOLFIELD(t, "mayAlias", false);
		depthDesc->clearDepth = SIF_OPTFLOATFIELD(t, "clearDepth", 1.f);
		depthDesc->clearStencil = SIF_OPTINTFIELD(t, "clearStencil", 0);
	}
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(1, "input");
	const uint32_t inCount = lua_istable(vm, t) ? (uint32_t)lua_rawlen(vm, t) : 0;
	struct NeAttachmentDesc *inDesc = NULL;
	if (lua_istable(vm, t)) {
		outDesc = Sys_Alloc(sizeof(*outDesc), outCount, MH_Transient);
		for (lua_Integer i = 0; i < outCount; ++i) {
			lua_rawgeti(vm, t, i + 1);
			int v = lua_gettop(vm);

			int f = SIF_GETFIELD(v, "format");
			if (lua_isinteger(vm, f)) {
				outDesc[i].format = (enum NeTextureFormat)lua_tointeger(vm, f);
			} else {
				size_t len;
				const char *str = luaL_checklstring(vm, f, &len);

				if (!strncmp("swapchain", str, len))
					outDesc[i].format = Re_SwapchainFormat(Re_swapchain);
				else
					luaL_argerror(vm, f, "format must be a TextureFormat or \"swapchain\"");
			}
			SIF_POPFIELD(f);

			inDesc[i].layout = SIF_OPTINTFIELD(v, "layout", TL_COLOR_ATTACHMENT);
			inDesc[i].initialLayout = SIF_OPTINTFIELD(v, "initialLayout", TL_COLOR_ATTACHMENT);
			inDesc[i].finalLayout = SIF_OPTINTFIELD(v, "finalLayout", TL_COLOR_ATTACHMENT);
			inDesc[i].samples = SIF_OPTINTFIELD(v, "samples", 1);
			inDesc[i].loadOp = SIF_OPTINTFIELD(v, "load", ATL_LOAD);
			inDesc[i].storeOp = SIF_OPTINTFIELD(v, "store", ATS_STORE);
			inDesc[i].mayAlias = SIF_OPTBOOLFIELD(v, "mayAlias", false);

			lua_remove(vm, v);
		}
	}
	SIF_POPFIELD(t);

	struct NeScriptWrapper *sw = lua_newuserdatauv(vm, sizeof(*sw), 0);
	sw->ptr = Re_CreateRenderPassDesc(outDesc, outCount, depthDesc, inDesc, inCount);
	luaL_setmetatable(vm, SIF_NE_RENDER_PASS_DESC);

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

static void
RegisterRenderPass(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		{ "Destroy", Sif_DestroyRenderPassDesc },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_RENDER_PASS_DESC, Sif_NewRenderPassDesc);
	luaL_newmetatable(vm, SIF_NE_RENDER_PASS_DESC);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);
}

// Buffer
SIF_FUNC(NewBuffer)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeBufferCreateInfo *bci = Sys_Alloc(sizeof(*bci), 1, MH_Transient);

	bci->desc.size = SIF_INTFIELD(1, "size");
	bci->desc.usage = SIF_INTFIELD(1, "usage");
	bci->desc.memoryType = SIF_OPTINTFIELD(1, "memoryType", MT_GPU_LOCAL);
	bci->desc.name = SIF_OPTSTRINGFIELD(1, "name", NULL);

	NeBufferHandle h;
	if (!Re_CreateBuffer(bci, &h))
		luaL_error(vm , "Failed to create buffer %s size = %d, usage = %x, memoryType = %x",
					bci->desc.name ? bci->desc.name : "unnamed", bci->desc.size, bci->desc.usage, bci->desc.memoryType);

	struct ScBuffer *sb = lua_newuserdatauv(vm, sizeof(*sb), 0);
	sb->buff = NULL;
	sb->handle = h;
	luaL_setmetatable(vm, SIF_NE_BUFFER);

	return 1;
}

SIF_FUNC(NewTransientBuffer)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeBufferDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);
	desc->memoryType = MT_GPU_LOCAL;

	desc->size = SIF_INTFIELD(1, "size");
	desc->usage = SIF_INTFIELD(1, "usage");
	desc->name = SIF_OPTSTRINGFIELD(1, "name", NULL);
	NeBufferHandle h = SIF_OPTINTFIELD(1, "handle", 0);

	struct NeBuffer *buff = Re_CreateTransientBuffer(desc, h);
	if (!buff)
		luaL_error(vm , "Failed to create transient buffer %s, size = %d, usage = %x",
				   desc->name ? desc->name : "unnamed", desc->size, desc->usage);

	struct ScBuffer *sb = lua_newuserdatauv(vm, sizeof(*sb), 0);
	sb->buff = buff;
	sb->handle = h;
	luaL_setmetatable(vm, SIF_NE_BUFFER);

	return 1;
}

SIF_FUNC(DestroyBuffer)
{
	struct ScBuffer *sb = luaL_checkudata(vm, 1, SIF_NE_BUFFER);
	Re_Destroy(sb->handle);
	return 0;
}

static void
RegisterBuffer(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Attachment),
		{ "Destroy", Sif_DestroyBuffer },
		SIF_ENDREG()
	};

	lua_register(vm, SIF_NE_BUFFER, Sif_NewBuffer);
	lua_register(vm, "NeTransientBuffer", Sif_NewTransientBuffer);

	luaL_newmetatable(vm, SIF_NE_BUFFER);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, BU_TRANSFER_SRC);
		lua_setfield(vm, -2, "TransferSource");
		lua_pushinteger(vm, BU_TRANSFER_DST);
		lua_setfield(vm, -2, "TransferDestination");
		lua_pushinteger(vm, BU_UNIFORM_BUFFER);
		lua_setfield(vm, -2, "Uniform");
		lua_pushinteger(vm, BU_STORAGE_BUFFER);
		lua_setfield(vm, -2, "Storage");
		lua_pushinteger(vm, BU_INDEX_BUFFER);
		lua_setfield(vm, -2, "Index");
		lua_pushinteger(vm, BU_VERTEX_BUFFER);
		lua_setfield(vm, -2, "Vertex");
		lua_pushinteger(vm, BU_INDIRECT_BUFFER);
		lua_setfield(vm, -2, "Indirect");
		lua_pushinteger(vm, BU_AS_BUILD_INPUT);
		lua_setfield(vm, -2, "AccelerationStructureBuild");
		lua_pushinteger(vm, BU_AS_STORAGE);
		lua_setfield(vm, -2, "AccelerationStructure");
		lua_pushinteger(vm, BU_SHADER_BINDING_TABLE);
		lua_setfield(vm, -2, "ShaderBindingTable");
	}
	lua_setglobal(vm, "BufferUsage");
}

// Texture
SIF_FUNC(NewTransientTexture)
{
	luaL_checktype(vm, 1, LUA_TTABLE);

	struct NeTextureDesc *desc = Sys_Alloc(sizeof(*desc), 1, MH_Transient);
	desc->width = SIF_INTFIELD(3, "width");
	desc->height = SIF_INTFIELD(3, "height");
	desc->type = SIF_INTFIELD(3, "type");
	desc->usage = SIF_INTFIELD(3, "usage");
	desc->format = SIF_INTFIELD(3, "format");
	desc->arrayLayers = SIF_OPTINTFIELD(3, "arrayLayers", 1);
	desc->mipLevels = SIF_OPTINTFIELD(3, "mipLevels", 1);
	desc->memoryType = MT_GPU_LOCAL;
	desc->gpuOptimalTiling = true;
	desc->name = SIF_OPTSTRINGFIELD(1, "name", NULL);

	uint16_t location = SIF_OPTINTFIELD(1, "handle", 0);

	struct NeTexture *tex = Re_CreateTransientTexture(desc, location);
	if (!tex)
		luaL_error(vm , "Failed to create transient texture %s, width = %d, height = %d, format = %x, usage = %x",
				   desc->name ? desc->name : "unnamed", desc->width, desc->height, desc->format, desc->usage);

	struct ScTexture *st = lua_newuserdatauv(vm, sizeof(*st), 0);
	st->tex = tex;
	st->location = location;
	memcpy(&st->desc, desc, sizeof(st->desc));
	luaL_setmetatable(vm, SIF_NE_TEXTURE);

	return 1;
}

SIF_FUNC(TextureDesc)
{
	struct ScTexture *st = luaL_checkudata(vm, 1, SIF_NE_TEXTURE);

	lua_newtable(vm);
	lua_pushinteger(vm, st->desc.width);
	lua_setfield(vm, -2, "width");
	lua_pushinteger(vm, st->desc.height);
	lua_setfield(vm, -2, "height");
	lua_pushinteger(vm, st->desc.type);
	lua_setfield(vm, -2, "type");
	lua_pushinteger(vm, st->desc.usage);
	lua_setfield(vm, -2, "usage");
	lua_pushinteger(vm, st->desc.format);
	lua_setfield(vm, -2, "format");
	lua_pushinteger(vm, st->desc.arrayLayers);
	lua_setfield(vm, -2, "arrayLayers");
	lua_pushinteger(vm, st->desc.mipLevels);
	lua_setfield(vm, -2, "mipLevels");
	lua_pushinteger(vm, st->desc.memoryType);
	lua_setfield(vm, -2, "memoryType");
	lua_pushboolean(vm, st->desc.gpuOptimalTiling);
	lua_setfield(vm, -2, "gpuOptimalTiling");
	
	return 1;
}

SIF_FUNC(TextureLayout)
{
	struct ScTexture *st = luaL_checkudata(vm, 1, SIF_NE_TEXTURE);
	lua_pushinteger(vm, Re_TextureLayout(st->location));
	return 1;
}

SIF_FUNC(DestroyTexture)
{
	struct ScTexture *st = luaL_checkudata(vm, 1, SIF_NE_TEXTURE);
	Re_Destroy(st->tex);
	return 0;
}

static void
RegisterTexture(lua_State *vm)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		{ "Desc", Sif_TextureDesc },
		{ "Layout", Sif_TextureLayout },
		{ "Destroy", Sif_DestroyTexture },
		SIF_ENDREG()
	};

	lua_register(vm, "NeTransientTexture", Sif_NewTransientTexture);

	luaL_newmetatable(vm, SIF_NE_TEXTURE);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, TF_R8G8B8A8_UNORM);
		lua_setfield(vm, -2, "RGBA8_Unorm");
		lua_pushinteger(vm, TF_R8G8B8A8_SRGB);
		lua_setfield(vm, -2, "RGBA8_sRGB");
		lua_pushinteger(vm, TF_B8G8R8A8_UNORM);
		lua_setfield(vm, -2, "BGRA8_Unorm");
		lua_pushinteger(vm, TF_B8G8R8A8_SRGB);
		lua_setfield(vm, -2, "BGRA8_sRGB");
		lua_pushinteger(vm, TF_R8G8_UNORM);
		lua_setfield(vm, -2, "RG8_Unorm");
		lua_pushinteger(vm, TF_R8_UNORM);
		lua_setfield(vm, -2, "R8_Unorm");

		lua_pushinteger(vm, TF_R16G16B16A16_UNORM);
		lua_setfield(vm, -2, "RGBA16_Unorm");
		lua_pushinteger(vm, TF_R16G16B16A16_SFLOAT);
		lua_setfield(vm, -2, "RGBA16_SFloat");
		lua_pushinteger(vm, TF_R16G16_UNORM);
		lua_setfield(vm, -2, "RG16_Unorm");
		lua_pushinteger(vm, TF_R16_UNORM);
		lua_setfield(vm, -2, "R16_Unorm");

		lua_pushinteger(vm, TF_R32G32B32A32_UINT);
		lua_setfield(vm, -2, "RGBA32_Uint");
		lua_pushinteger(vm, TF_R32G32B32A32_SFLOAT);
		lua_setfield(vm, -2, "RGBA32_Sfloat");
		lua_pushinteger(vm, TF_R32G32_UINT);
		lua_setfield(vm, -2, "RG32_Uint");
		lua_pushinteger(vm, TF_R32_UINT);
		lua_setfield(vm, -2, "R32_Uint");

		lua_pushinteger(vm, TF_D32_SFLOAT);
		lua_setfield(vm, -2, "D32_Sfloat");
		lua_pushinteger(vm, TF_D24_STENCIL8);
		lua_setfield(vm, -2, "D23_S8");
		lua_pushinteger(vm, TF_A2R10G10B10_UNORM);
		lua_setfield(vm, -2, "ARGB10_Unorm");

		lua_pushinteger(vm, TF_BC5_UNORM);
		lua_setfield(vm, -2, "BC5_Unorm");
		lua_pushinteger(vm, TF_BC5_SNORM);
		lua_setfield(vm, -2, "BC5_Snorm");
		lua_pushinteger(vm, TF_BC6H_UF16);
		lua_setfield(vm, -2, "BC6H_Ufloat");
		lua_pushinteger(vm, TF_BC6H_SF16);
		lua_setfield(vm, -2, "BC6H_Ufloat");
		lua_pushinteger(vm, TF_BC7_UNORM);
		lua_setfield(vm, -2, "BC7_Unorm");
		lua_pushinteger(vm, TF_BC7_SRGB);
		lua_setfield(vm, -2, "BC7_sRGB");

		lua_pushinteger(vm, TF_ETC2_R8G8B8_UNORM);
		lua_setfield(vm, -2, "ETC2_RGB8_Unorm");
		lua_pushinteger(vm, TF_ETC2_R8G8B8_SRGB);
		lua_setfield(vm, -2, "ETC2_RGB8_sRGB");
		lua_pushinteger(vm, TF_ETC2_R8G8B8A1_UNORM);
		lua_setfield(vm, -2, "RTC2_RGB8A1_Unorm");
		lua_pushinteger(vm, TF_ETC2_R8G8B8A1_SRGB);
		lua_setfield(vm, -2, "RTC2_RGB8A1_sRGB");

		lua_pushinteger(vm, TF_EAC_R11_UNORM);
		lua_setfield(vm, -2, "EAC_R11_Unorm");
		lua_pushinteger(vm, TF_EAC_R11_SNORM);
		lua_setfield(vm, -2, "EAC_R11_Snorm");
		lua_pushinteger(vm, TF_EAC_R11G11_UNORM);
		lua_setfield(vm, -2, "EAC_RG11_Unorm");
		lua_pushinteger(vm, TF_EAC_R11G11_SNORM);
		lua_setfield(vm, -2, "EAC_RG11_Snorm");
	}
	lua_setglobal(vm, "TextureFormat");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, TU_TRANSFER_SRC);
		lua_setfield(vm, -2, "TransferSource");
		lua_pushinteger(vm, TU_TRANSFER_DST);
		lua_setfield(vm, -2, "TransferDestination");
		lua_pushinteger(vm, TU_SAMPLED);
		lua_setfield(vm, -2, "Sampled");
		lua_pushinteger(vm, TU_STORAGE);
		lua_setfield(vm, -2, "Storage");
		lua_pushinteger(vm, TU_COLOR_ATTACHMENT);
		lua_setfield(vm, -2, "Color");
		lua_pushinteger(vm, TU_DEPTH_STENCIL_ATTACHMENT);
		lua_setfield(vm, -2, "DepthStencil");
		lua_pushinteger(vm, TU_INPUT_ATTACHMENT);
		lua_setfield(vm, -2, "Input");
		lua_pushinteger(vm, TU_SHADING_RATE_ATTACHMENT);
		lua_setfield(vm, -2, "ShadingRate");
		lua_pushinteger(vm, TU_FRAGMENT_DENSITY_MAP);
		lua_setfield(vm, -2, "FragmentDensity");
	}
	lua_setglobal(vm, "TextureUsage");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, TL_UNKNOWN);
		lua_setfield(vm, -2, "Unknown");
		lua_pushinteger(vm, TL_COLOR_ATTACHMENT);
		lua_setfield(vm, -2, "Color");
		lua_pushinteger(vm, TL_DEPTH_STENCIL_ATTACHMENT);
		lua_setfield(vm, -2, "DepthStencil");
		lua_pushinteger(vm, TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT);
		lua_setfield(vm, -2, "DepthStencilReadOnly");
		lua_pushinteger(vm, TL_DEPTH_ATTACHMENT);
		lua_setfield(vm, -2, "Depth");
		lua_pushinteger(vm, TL_STENCIL_ATTACHMENT);
		lua_setfield(vm, -2, "Stencil");
		lua_pushinteger(vm, TL_DEPTH_READ_ONLY_ATTACHMENT);
		lua_setfield(vm, -2, "DepthReadOnly");
		lua_pushinteger(vm, TL_TRANSFER_SRC);
		lua_setfield(vm, -2, "TransferSource");
		lua_pushinteger(vm, TL_TRANSFER_DST);
		lua_setfield(vm, -2, "TransferDestination");
		lua_pushinteger(vm, TL_SHADER_READ_ONLY);
		lua_setfield(vm, -2, "ReadOnly");
		lua_pushinteger(vm, TL_PRESENT_SRC);
		lua_setfield(vm, -2, "PresentSource");
	}
	lua_setglobal(vm, "TextureLayout");
}

NE_SCRIPT_INTEFACE(NeRender)
{
	RegisterCommandBuffer(vm);
	RegisterRenderGraph(vm);
	RegisterFramebuffer(vm);
	RegisterRenderPass(vm);
	RegisterBuffer(vm);
	RegisterTexture(vm);

	lua_register(vm, "NeGraphicsPipeline", Sif_GraphicsPipeline);
	lua_register(vm, "NeComputePipeline", Sif_ComputePipeline);
	lua_register(vm, "NeRayTracingPipeline", Sif_RayTracingPipeline);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, SCBT_Graphics);
		lua_setglobal(vm, "Graphics");
		lua_pushinteger(vm, SCBT_Compute);
		lua_setglobal(vm, "Compute");
		lua_pushinteger(vm, SCBT_Transfer);
		lua_setglobal(vm, "Transfer");
	}
	lua_setglobal(vm, "CommandBufferType");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IA_COLOR);
		lua_setfield(vm, -2, "Color");
		lua_pushinteger(vm, IA_DEPTH);
		lua_setfield(vm, -2, "Depth");
		lua_pushinteger(vm, IA_STENCIL);
		lua_setfield(vm, -2, "Stencil");
	}
	lua_setglobal(vm, "ImageAspect");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, MT_GPU_LOCAL);
		lua_setfield(vm, -2, "DeviceLocal");
		lua_pushinteger(vm, MT_CPU_READ);
		lua_setfield(vm, -2, "ReadOnly");
		lua_pushinteger(vm, MT_CPU_WRITE);
		lua_setfield(vm, -2, "WriteOnly");
		lua_pushinteger(vm, MT_CPU_COHERENT);
		lua_setfield(vm, -2, "Coherent");
	}
	lua_setglobal(vm, "MemoryType");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_PD_BY_REGION);
		lua_setfield(vm, -2, "ByRegion");
		lua_pushinteger(vm, RE_PD_DEVICE_GROUP);
		lua_setfield(vm, -2, "DeviceGroup");
		lua_pushinteger(vm, RE_PD_VIEW_LOCAL);
		lua_setfield(vm, -2, "ViewLocal");
	}
	lua_setglobal(vm, "PipelineDependency");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IT_UINT_16);
		lua_setfield(vm, -2, "Uint16");
		lua_pushinteger(vm, IT_UINT_32);
		lua_setfield(vm, -2, "Uint32");
	}
	lua_setglobal(vm, "IndexType");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, PT_TRIANGLES);
		lua_setfield(vm, -2, "Triangles");
		lua_pushinteger(vm, PT_POINTS);
		lua_setfield(vm, -2, "Points");
		lua_pushinteger(vm, PT_LINES);
		lua_setfield(vm, -2, "Lines");
	}
	lua_setglobal(vm, "PrimitiveType");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, SS_VERTEX);
		lua_setfield(vm, -2, "Vertex");
		lua_pushinteger(vm, SS_TESS_CTRL);
		lua_setfield(vm, -2, "TessellationControl");
		lua_pushinteger(vm, SS_TESS_EVAL);
		lua_setfield(vm, -2, "TessellationEvaluation");
		lua_pushinteger(vm, SS_GEOMETRY);
		lua_setfield(vm, -2, "Geometry");
		lua_pushinteger(vm, SS_FRAGMENT);
		lua_setfield(vm, -2, "Fragment");
		lua_pushinteger(vm, SS_COMPUTE);
		lua_setfield(vm, -2, "Compute");
		lua_pushinteger(vm, SS_ALL_GRAPHICS);
		lua_setfield(vm, -2, "AllGraphics");
		lua_pushinteger(vm, SS_RAYGEN);
		lua_setfield(vm, -2, "RayGeneration");
		lua_pushinteger(vm, SS_ANY_HIT);
		lua_setfield(vm, -2, "AnyHit");
		lua_pushinteger(vm, SS_CLOSEST_HIT);
		lua_setfield(vm, -2, "ClosestHit");
		lua_pushinteger(vm, SS_MISS);
		lua_setfield(vm, -2, "Miss");
		lua_pushinteger(vm, SS_INTERSECTION);
		lua_setfield(vm, -2, "Intersection");
		lua_pushinteger(vm, SS_CALLABLE);
		lua_setfield(vm, -2, "Callable");
		lua_pushinteger(vm, SS_TASK);
		lua_setfield(vm, -2, "Task");
		lua_pushinteger(vm, SS_MESH);
		lua_setfield(vm, -2, "Mesh");
		lua_pushinteger(vm, SS_ALL);
		lua_setfield(vm, -2, "All");
	}
	lua_setglobal(vm, "ShaderStage");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_POLYGON_FILL);
		lua_setfield(vm, -2, "Fill");
		lua_pushinteger(vm, RE_POLYGON_LINE);
		lua_setfield(vm, -2, "Line");
		lua_pushinteger(vm, RE_POLYGON_POINT);
		lua_setfield(vm, -2, "Point");
	}
	lua_setglobal(vm, "FillMode");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_CULL_NONE);
		lua_setfield(vm, -2, "None");
		lua_pushinteger(vm, RE_CULL_BACK);
		lua_setfield(vm, -2, "Back");
		lua_pushinteger(vm, RE_CULL_FRONT);
		lua_setfield(vm, -2, "Front");
	}
	lua_setglobal(vm, "CullMode");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_FRONT_FACE_CCW);
		lua_setfield(vm, -2, "CounterClockwise");
		lua_pushinteger(vm, RE_FRONT_FACE_CW);
		lua_setfield(vm, -2, "Clockwise");
	}
	lua_setglobal(vm, "FrontFace");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_WRITE_MASK_R);
		lua_setfield(vm, -2, "Red");
		lua_pushinteger(vm, RE_WRITE_MASK_G);
		lua_setfield(vm, -2, "Green");
		lua_pushinteger(vm, RE_WRITE_MASK_B);
		lua_setfield(vm, -2, "Blue");
		lua_pushinteger(vm, RE_WRITE_MASK_A);
		lua_setfield(vm, -2, "Alpha");
		lua_pushinteger(vm, RE_WRITE_MASK_RGB);
		lua_setfield(vm, -2, "RGB");
		lua_pushinteger(vm, RE_WRITE_MASK_RGBA);
		lua_setfield(vm, -2, "RGBA");
	}
	lua_setglobal(vm, "WriteMask");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, VIR_VERTEX);
		lua_setfield(vm, -2, "Vertex");
		lua_pushinteger(vm, VIR_INSTANCE);
		lua_setfield(vm, -2, "Instance");
	}
	lua_setglobal(vm, "InputRate");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, VF_FLOAT);
		lua_setfield(vm, -2, "Float");
		lua_pushinteger(vm, VF_FLOAT2);
		lua_setfield(vm, -2, "Float2");
		lua_pushinteger(vm, VF_FLOAT3);
		lua_setfield(vm, -2, "Float3");
		lua_pushinteger(vm, VF_FLOAT4);
		lua_setfield(vm, -2, "Float4");
	}
	lua_setglobal(vm, "VertexFormat");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, AS_POSITION);
		lua_setfield(vm, -2, "Position");
		lua_pushinteger(vm, AS_BLENDWEIGHT);
		lua_setfield(vm, -2, "BlendWeight");
		lua_pushinteger(vm, AS_BLENDINDICES);
		lua_setfield(vm, -2, "BlendIndices");
		lua_pushinteger(vm, AS_NORMAL);
		lua_setfield(vm, -2, "Normal");
		lua_pushinteger(vm, AS_TEXCOORD0);
		lua_setfield(vm, -2, "TexCoord0");
		lua_pushinteger(vm, AS_TEXCOORD1);
		lua_setfield(vm, -2, "TexCoord1");
		lua_pushinteger(vm, AS_TANGENT);
		lua_setfield(vm, -2, "Tangent");
		lua_pushinteger(vm, AS_BINORMAL);
		lua_setfield(vm, -2, "BiNormal");
		lua_pushinteger(vm, AS_TESSFACTOR);
		lua_setfield(vm, -2, "TessFactor");
		lua_pushinteger(vm, AS_COLOR);
		lua_setfield(vm, -2, "Color");
	}
	lua_setglobal(vm, "VertexSemantic");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_BF_ZERO);
		lua_setfield(vm, -2, "Zero");
		lua_pushinteger(vm, RE_BF_ONE);
		lua_setfield(vm, -2, "One");
		lua_pushinteger(vm, RE_BF_SRC_COLOR);
		lua_setfield(vm, -2, "SrcColor");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_SRC_COLOR);
		lua_setfield(vm, -2, "OneMinusSrcColor");
		lua_pushinteger(vm, RE_BF_DST_COLOR);
		lua_setfield(vm, -2, "DstColor");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_DST_COLOR);
		lua_setfield(vm, -2, "OneMinusDstColor");
		lua_pushinteger(vm, RE_BF_SRC_ALPHA);
		lua_setfield(vm, -2, "SrcAlpha");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_SRC_ALPHA);
		lua_setfield(vm, -2, "OneMinusSrcAlpha");
		lua_pushinteger(vm, RE_BF_DST_ALPHA);
		lua_setfield(vm, -2, "DstAlpha");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_DST_ALPHA);
		lua_setfield(vm, -2, "OneMinusDstAlpha");
		lua_pushinteger(vm, RE_BF_CONSTANT_COLOR);
		lua_setfield(vm, -2, "ConstantColor");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_CONSTANT_COLOR);
		lua_setfield(vm, -2, "OneMinusConstantColor");
		lua_pushinteger(vm, RE_BF_CONSTANT_ALPHA);
		lua_setfield(vm, -2, "ConstantAlpha");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_CONSTANT_ALPHA);
		lua_setfield(vm, -2, "OneMinusConstantAlpha");
		lua_pushinteger(vm, RE_BF_SRC_ALPHA_SATURATE);
		lua_setfield(vm, -2, "SrcAlphaSaturate");
		lua_pushinteger(vm, RE_BF_SRC1_COLOR);
		lua_setfield(vm, -2, "Src1Color");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_SRC1_COLOR);
		lua_setfield(vm, -2, "OneMinusSrc1Color");
		lua_pushinteger(vm, RE_BF_SRC1_ALPHA);
		lua_setfield(vm, -2, "Src1Alpha");
		lua_pushinteger(vm, RE_BF_ONE_MINUS_SRC1_ALPHA);
		lua_setfield(vm, -2, "OneMinusSrc1Alpha");
	}
	lua_setglobal(vm, "BlendFactor");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_BOP_ADD);
		lua_setfield(vm, -2, "Add");
		lua_pushinteger(vm, RE_BOP_SUBTRACT);
		lua_setfield(vm, -2, "Subtract");
		lua_pushinteger(vm, RE_BOP_REVERSE_SUBTRACT);
		lua_setfield(vm, -2, "ReverseSubtract");
		lua_pushinteger(vm, RE_BOP_MIN);
		lua_setfield(vm, -2, "Min");
		lua_pushinteger(vm, RE_BOP_MAX);
		lua_setfield(vm, -2, "Max");
	}
	lua_setglobal(vm, "BlendOp");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, ATL_LOAD);
		lua_setfield(vm, -2, "Load");
		lua_pushinteger(vm, ATL_CLEAR);
		lua_setfield(vm, -2, "Clear");
		lua_pushinteger(vm, ATL_DONT_CARE);
		lua_setfield(vm, -2, "DontCare");
	}
	lua_setglobal(vm, "LoadOp");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, ATS_STORE);
		lua_setfield(vm, -2, "Store");
		lua_pushinteger(vm, ATS_DONT_CARE);
		lua_setfield(vm, -2, "DontCare");
	}
	lua_setglobal(vm, "StoreOp");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, CO_NEVER);
		lua_setfield(vm, -2, "Never");
		lua_pushinteger(vm, CO_LESS);
		lua_setfield(vm, -2, "Less");
		lua_pushinteger(vm, CO_EQUAL);
		lua_setfield(vm, -2, "Equal");
		lua_pushinteger(vm, CO_LESS_EQUAL);
		lua_setfield(vm, -2, "LessEqual");
		lua_pushinteger(vm, CO_GREATER);
		lua_setfield(vm, -2, "Greater");
		lua_pushinteger(vm, CO_NOT_EQUAL);
		lua_setfield(vm, -2, "NotEqual");
		lua_pushinteger(vm, CO_GREATER_EQUAL);
		lua_setfield(vm, -2, "GreaterEqual");
		lua_pushinteger(vm, CO_ALWAYS);
		lua_setfield(vm, -2, "Always");
	}
	lua_setglobal(vm, "CompareOp");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IF_NEAREST);
		lua_setfield(vm, -2, "Nearest");
		lua_pushinteger(vm, IF_LINEAR);
		lua_setfield(vm, -2, "Linear");
		lua_pushinteger(vm, IF_CUBIC);
		lua_setfield(vm, -2, "Cubic");
	}
	lua_setglobal(vm, "ImageFilter");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_QUEUE_GRAPHICS);
		lua_setfield(vm, -2, "Graphics");
		lua_pushinteger(vm, RE_QUEUE_TRANSFER);
		lua_setfield(vm, -2, "Transfer");
		lua_pushinteger(vm, RE_QUEUE_COMPUTE);
		lua_setfield(vm, -2, "Compute");
	}
	lua_setglobal(vm, "RenderQueue");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RS_PS_NONE);
		lua_setfield(vm, -2, "None");
		lua_pushinteger(vm, RE_PS_TOP_OF_PIPE);
		lua_setfield(vm, -2, "Top");
		lua_pushinteger(vm, RE_PS_DRAW_INDIRECT);
		lua_setfield(vm, -2, "IndirectDraw");
		lua_pushinteger(vm, RE_PS_VERTEX_INPUT);
		lua_setfield(vm, -2, "VertexInput");
		lua_pushinteger(vm, RE_PS_VERTEX_SHADER);
		lua_setfield(vm, -2, "VertexShaderShader");
		lua_pushinteger(vm, RE_PS_TESSELLATION_CONTROL_SHADER);
		lua_setfield(vm, -2, "TessellationControlShader");
		lua_pushinteger(vm, RE_PS_TESSELLATION_EVALUATION_SHADER);
		lua_setfield(vm, -2, "TessellationEvaluationShader");
		lua_pushinteger(vm, RE_PS_GEOMETRY_SHADER);
		lua_setfield(vm, -2, "GeometryShader");
		lua_pushinteger(vm, RE_PS_FRAGMENT_SHADER);
		lua_setfield(vm, -2, "FragmentShader");
		lua_pushinteger(vm, RE_PS_EARLY_FRAGMENT_TESTS);
		lua_setfield(vm, -2, "EarlyFragmentTests");
		lua_pushinteger(vm, RE_PS_LATE_FRAGMENT_TESTS);
		lua_setfield(vm, -2, "LateFragmentTests");
		lua_pushinteger(vm, RE_PS_COLOR_ATTACHMENT_OUTPUT);
		lua_setfield(vm, -2, "ColorAttachmentOutput");
		lua_pushinteger(vm, RE_PS_COMPUTE_SHADER);
		lua_setfield(vm, -2, "ComputeShader");
		lua_pushinteger(vm, RE_PS_TRANSFER);
		lua_setfield(vm, -2, "Transfoer");
		lua_pushinteger(vm, RE_PS_BOTTOM_OF_PIPE);
		lua_setfield(vm, -2, "Bottom");
		lua_pushinteger(vm, RE_PS_HOST);
		lua_setfield(vm, -2, "Host");
		lua_pushinteger(vm, RE_PS_ALL_GRAPHICS);
		lua_setfield(vm, -2, "AllGraphics");
		lua_pushinteger(vm, RE_PS_ALL_COMMANDS);
		lua_setfield(vm, -2, "AllCommands");
		lua_pushinteger(vm, RE_PS_COPY);
		lua_setfield(vm, -2, "Copy");
		lua_pushinteger(vm, RE_PS_RESOLVE);
		lua_setfield(vm, -2, "Resolve");
		lua_pushinteger(vm, RE_PS_BLIT);
		lua_setfield(vm, -2, "Blit");
		lua_pushinteger(vm, RE_PS_CLEAR);
		lua_setfield(vm, -2, "Clear");
		lua_pushinteger(vm, RE_PS_INDEX_INPUT);
		lua_setfield(vm, -2, "IndexInput");
		lua_pushinteger(vm, RE_PS_VERTEX_ATTRIBUTE_INPUT);
		lua_setfield(vm, -2, "VertexAttributeInput");
		lua_pushinteger(vm, RE_PS_PRE_RASTERIZATION_SHADERS);
		lua_setfield(vm, -2, "PreRasterizationShaders");
		lua_pushinteger(vm, RE_PS_TRANSFORM_FEEDBACK);
		lua_setfield(vm, -2, "TransformFeedback");
		lua_pushinteger(vm, RE_PS_CONDITIONAL_RENDERING);
		lua_setfield(vm, -2, "ConditionalRendering");
		lua_pushinteger(vm, RE_PS_COMMAND_PREPROCESS);
		lua_setfield(vm, -2, "CommandPreprocess");
		lua_pushinteger(vm, RE_PS_FRAGMENT_SHADING_RATE_ATTACHMENT);
		lua_setfield(vm, -2, "FragmentShadingRateAttachment");
		lua_pushinteger(vm, RE_PS_ACCELERATION_STRUCTURE_BUILD);
		lua_setfield(vm, -2, "AccelerationStructureBuild");
		lua_pushinteger(vm, RE_PS_RAY_TRACING_SHADER);
		lua_setfield(vm, -2, "RayTracingShader");
		lua_pushinteger(vm, RE_PS_FRAGMENT_DENSITY_PROCESS);
		lua_setfield(vm, -2, "FragmentDensityProcess");
		lua_pushinteger(vm, RE_PS_TASK_SHADER_BIT);
		lua_setfield(vm, -2, "TaskShader");
		lua_pushinteger(vm, RE_PS_MESH_SHADER_BIT);
		lua_setfield(vm, -2, "MeshShader");
	}
	lua_setglobal(vm, "PipelineStage");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, RE_PA_NONE);
		lua_setfield(vm, -2, "None");
		lua_pushinteger(vm, RE_PA_INDIRECT_COMMAND_READ);
		lua_setfield(vm, -2, "IndirectCommandRead");
		lua_pushinteger(vm, RE_PA_INDEX_READ);
		lua_setfield(vm, -2, "IndexRead");
		lua_pushinteger(vm, RE_PA_VERTEX_ATTRIBUTE_READ);
		lua_setfield(vm, -2, "VertexAttributeRead");
		lua_pushinteger(vm, RE_PA_UNIFORM_READ);
		lua_setfield(vm, -2, "UniformRead");
		lua_pushinteger(vm, RE_PA_INPUT_ATTACHMENT_READ);
		lua_setfield(vm, -2, "InputAttachmentRead");
		lua_pushinteger(vm, RE_PA_SHADER_READ);
		lua_setfield(vm, -2, "ShaderRead");
		lua_pushinteger(vm, RE_PA_SHADER_WRITE);
		lua_setfield(vm, -2, "ShaderWrite");
		lua_pushinteger(vm, RE_PA_COLOR_ATTACHMENT_READ);
		lua_setfield(vm, -2, "ColorAttachmentRead");
		lua_pushinteger(vm, RE_PA_COLOR_ATTACHMENT_WRITE);
		lua_setfield(vm, -2, "ColorAttachmentWrite");
		lua_pushinteger(vm, RE_PA_DEPTH_STENCIL_ATTACHMENT_READ);
		lua_setfield(vm, -2, "DepthStencilAttachmentRead");
		lua_pushinteger(vm, RE_PA_DEPTH_STENCIL_ATTACHMENT_WRITE);
		lua_setfield(vm, -2, "DepthStencilAttachmentWrite");
		lua_pushinteger(vm, RE_PA_TRANSFER_READ);
		lua_setfield(vm, -2, "TransferRead");
		lua_pushinteger(vm, RE_PA_TRANSFER_WRITE);
		lua_setfield(vm, -2, "TransferWrite");
		lua_pushinteger(vm, RE_PA_HOST_READ);
		lua_setfield(vm, -2, "HostRead");
		lua_pushinteger(vm, RE_PA_HOST_WRITE);
		lua_setfield(vm, -2, "HostWrite");
		lua_pushinteger(vm, RE_PA_MEMORY_READ);
		lua_setfield(vm, -2, "MemoryRead");
		lua_pushinteger(vm, RE_PA_MEMORY_WRITE);
		lua_setfield(vm, -2, "MemoryWrite");
		lua_pushinteger(vm, RE_PA_SHADER_SAMPLED_READ);
		lua_setfield(vm, -2, "ShaderSampledRead");
		lua_pushinteger(vm, RE_PA_SHADER_STORAGE_READ);
		lua_setfield(vm, -2, "ShaderStorageRead");
		lua_pushinteger(vm, RE_PA_SHADER_STORAGE_WRITE);
		lua_setfield(vm, -2, "ShaderStorageWrite");
		lua_pushinteger(vm, RE_PA_TRANSFORM_FEEDBACK_WRITE);
		lua_setfield(vm, -2, "TransformFeedbackWrite");
		lua_pushinteger(vm, RE_PA_TRANSFORM_FEEDBACK_COUNTER_READ);
		lua_setfield(vm, -2, "TransformFeedbackCounterRead");
		lua_pushinteger(vm, RE_PA_TRANSFORM_FEEDBACK_COUNTER_WRITE);
		lua_setfield(vm, -2, "TransformFeedbackCounterWrite");
		lua_pushinteger(vm, RE_PA_CONDITIONAL_RENDERING_READ);
		lua_setfield(vm, -2, "ConditionalRenderingRead");
		lua_pushinteger(vm, RE_PA_COMMAND_PREPROCESS_READ);
		lua_setfield(vm, -2, "CommandPreprocessRead");
		lua_pushinteger(vm, RE_PA_COMMAND_PREPROCESS_WRITE);
		lua_setfield(vm, -2, "CommandPreprocessWrite");
		lua_pushinteger(vm, RE_PA_FRAGMENT_SHADING_RATE_ATTACHMENT_READ);
		lua_setfield(vm, -2, "FragmentShadingRateAttachmentRead");
		lua_pushinteger(vm, RE_PA_SHADING_RATE_IMAGE_READ);
		lua_setfield(vm, -2, "ShadingRateImageRead");
		lua_pushinteger(vm, RE_PA_ACCELERATION_STRUCTURE_READ);
		lua_setfield(vm, -2, "AccelerationStructureRead");
		lua_pushinteger(vm, RE_PA_ACCELERATION_STRUCTURE_WRITE);
		lua_setfield(vm, -2, "AccelerationStructureWrite");
		lua_pushinteger(vm, RE_PA_FRAGMENT_DENSITY_MAP_READ);
		lua_setfield(vm, -2, "FragmentDensityMapRead");
		lua_pushinteger(vm, RE_PA_COLOR_ATTACHMENT_READ_NONCOHERENT);
		lua_setfield(vm, -2, "ColorAttachmentReadNonCoherent");
	};
	lua_setglobal(vm, "PipelineAccess");

	return 1;
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
