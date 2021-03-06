#include <Runtime/Runtime.h>
#include <Render/Pipeline.h>

#define P_GRAPHICS		0
#define P_COMPUTE		1
#define P_RAY_TRACING	2

static struct Array _pipelines;

struct PipelineInfo
{
	uint32_t type;
	union {
		struct {
			struct Shader *sh;
			uint64_t flags;
			struct BlendAttachmentDesc *at;
			uint32_t atCount;
		} graphics;
		struct {
			struct Shader *sh;
		} compute;
		struct {
			struct ShaderBindingTable *sbt;
			uint32_t maxDepth;
		} rayTracing;
	};
	struct Pipeline *pipeline;
};

bool
Re_InitPipelines(void)
{
	if (!Rt_InitArray(&_pipelines, 10, sizeof(struct PipelineInfo)))
		return false;
	
	Re_deviceProcs.LoadPipelineCache(Re_device);
	
	return true;
}

struct Pipeline *Re_GraphicsPipeline(const struct GraphicsPipelineDesc *desc)
{
	struct PipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type != P_GRAPHICS ||
				pi->graphics.sh != desc->shader ||
				pi->graphics.flags != desc->flags ||
				pi->graphics.atCount != desc->attachmentCount)
			continue;
		
		bool ok = true;
		for (uint32_t i = 0; i < desc->attachmentCount; ++i)
			if (memcmp(&desc->attachments[i], &pi->graphics.at[i], sizeof(*desc->attachments)))
				ok = false;
		
		if (!ok)
			continue;
		
		return pi->pipeline;
	}
	
	struct BlendAttachmentDesc *newAt = calloc(desc->attachmentCount, sizeof(*newAt));
	if (!newAt)
		return NULL;
	
	memcpy(newAt, desc->attachments, desc->attachmentCount * sizeof(*newAt));
	
	struct PipelineInfo new =
	{
		.type = P_GRAPHICS,
		.graphics.sh = desc->shader,
		.graphics.flags = desc->flags,
		.graphics.at = newAt,
		.graphics.atCount = desc->attachmentCount,
		.pipeline = Re_deviceProcs.GraphicsPipeline(Re_device, desc)
	};
	
	if (!new.pipeline)
		return NULL;
	
	Rt_ArrayAdd(&_pipelines, &new);
	
	return new.pipeline;
}

struct Pipeline *Re_ComputePipeline(struct Shader *sh)
{
	struct PipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type != P_GRAPHICS || pi->compute.sh != sh)
			continue;
		
		return pi->pipeline;
	}
	
	struct PipelineInfo new =
	{
		.type = P_COMPUTE,
		.compute.sh = sh,
		.pipeline = Re_deviceProcs.ComputePipeline(Re_device, sh)
	};
	
	if (!new.pipeline)
		return NULL;
	
	Rt_ArrayAdd(&_pipelines, &new);
	
	return new.pipeline;
}

struct Pipeline *Re_RayTracingPipeline(struct ShaderBindingTable *sbt, uint32_t maxDepth)
{
	struct PipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type != P_RAY_TRACING || pi->rayTracing.sbt != sbt || pi->rayTracing.maxDepth != maxDepth)
			continue;
		
		return pi->pipeline;
	}
	
	struct PipelineInfo new =
	{
		.type = P_RAY_TRACING,
		.rayTracing.sbt = sbt,
		.rayTracing.maxDepth = maxDepth,
		.pipeline = Re_deviceProcs.RayTracingPipeline(Re_device, sbt, maxDepth)
	};
	
	if (!new.pipeline)
		return NULL;
	
	Rt_ArrayAdd(&_pipelines, &new);
	
	return new.pipeline;
}

void
Re_TermPipelines(void)
{
	struct PipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines)
		if (pi->type == P_GRAPHICS)
			free(pi->graphics.at);
	
	Re_deviceProcs.SavePipelineCache(Re_device);
	Rt_TermArray(&_pipelines);
}
