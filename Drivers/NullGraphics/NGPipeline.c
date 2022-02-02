#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>

#include "NullGraphicsDriver.h"

struct NePipeline *
NG_GraphicsPipeline(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(1, sizeof(*p), MH_RenderDriver);
	if (!p)
		return NULL;

	return p;
}

struct NePipeline *
NG_ComputePipeline(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	if (!p)
		return NULL;

	return p;
}

struct NePipeline *
NG_RayTracingPipeline(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth)
{
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	if (!p)
		return NULL;

	return p;
}

void
NG_LoadPipelineCache(struct NeRenderDevice *dev)
{
}

void
NG_SavePipelineCache(struct NeRenderDevice *dev)
{
}

void
NG_DestroyPipeline(struct NeRenderDevice *dev, struct NePipeline *pipeline)
{
	Sys_Free(pipeline);
}
