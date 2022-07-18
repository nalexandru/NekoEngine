#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Runtime/Runtime.h>

#define P_GRAPHICS		0
#define P_COMPUTE		1
#define P_RAY_TRACING	2

static struct NeArray _pipelines;

struct NePipelineInfo
{
	uint32_t type;
	union {
		struct {
			struct NeShaderStageInfo *stageInfo;
			uint64_t flags;
			struct NeBlendAttachmentDesc *at;
			uint32_t atCount;
		} graphics;
		struct {
			struct NeShaderStageInfo *stageInfo;
			struct {
				uint32_t x;
				uint32_t y;
				uint32_t z;
			} threadsPerThreadgroup;
		} compute;
		struct {
			struct NeShaderBindingTable *sbt;
			uint32_t maxDepth;
		} rayTracing;
	};
	struct NePipeline *pipeline;
};

bool
Re_InitPipelines(void)
{
	if (!Rt_InitArray(&_pipelines, 10, sizeof(struct NePipelineInfo), MH_Render))
		return false;

	Re_LoadPipelineCache();

	return true;
}

struct NePipeline *
Re_GraphicsPipeline(const struct NeGraphicsPipelineDesc *desc)
{
	struct NePipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type != P_GRAPHICS ||
				pi->graphics.stageInfo != desc->stageInfo ||
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

	struct NeBlendAttachmentDesc *newAt = Sys_Alloc(desc->attachmentCount, sizeof(*newAt), MH_Render);
	if (!newAt)
		return NULL;

	memcpy(newAt, desc->attachments, desc->attachmentCount * sizeof(*newAt));

	struct NePipelineInfo new =
	{
		.type = P_GRAPHICS,
		.graphics.stageInfo = desc->stageInfo,
		.graphics.flags = desc->flags,
		.graphics.at = newAt,
		.graphics.atCount = desc->attachmentCount,
		.pipeline = Re_BkGraphicsPipeline(desc)
	};

	if (!new.pipeline)
		return NULL;

	Rt_ArrayAdd(&_pipelines, &new);

	return new.pipeline;
}

struct NePipeline *
Re_ComputePipeline(const struct NeComputePipelineDesc *desc)
{
	struct NePipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type != P_GRAPHICS ||
				pi->compute.stageInfo != desc->stageInfo ||
				pi->compute.threadsPerThreadgroup.x != desc->threadsPerThreadgroup.x ||
				pi->compute.threadsPerThreadgroup.y != desc->threadsPerThreadgroup.y ||
				pi->compute.threadsPerThreadgroup.z != desc->threadsPerThreadgroup.z)
			continue;

		return pi->pipeline;
	}

	struct NePipelineInfo new =
	{
		.type = P_COMPUTE,
		.compute.stageInfo = desc->stageInfo,
		.compute.threadsPerThreadgroup.x = desc->threadsPerThreadgroup.x,
		.compute.threadsPerThreadgroup.y = desc->threadsPerThreadgroup.y,
		.compute.threadsPerThreadgroup.z = desc->threadsPerThreadgroup.z,
		.pipeline = Re_BkComputePipeline(desc)
	};

	if (!new.pipeline)
		return NULL;

	Rt_ArrayAdd(&_pipelines, &new);

	return new.pipeline;
}

struct NePipeline *
Re_RayTracingPipeline(struct NeShaderBindingTable *sbt, uint32_t maxDepth)
{
	struct NePipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type != P_RAY_TRACING || pi->rayTracing.sbt != sbt || pi->rayTracing.maxDepth != maxDepth)
			continue;

		return pi->pipeline;
	}

	struct NePipelineInfo new =
	{
		.type = P_RAY_TRACING,
		.rayTracing.sbt = sbt,
		.rayTracing.maxDepth = maxDepth,
		.pipeline = Re_BkRayTracingPipeline(sbt, maxDepth)
	};

	if (!new.pipeline)
		return NULL;

	Rt_ArrayAdd(&_pipelines, &new);

	return new.pipeline;
}

void
Re_TermPipelines(void)
{
	struct NePipelineInfo *pi;
	Rt_ArrayForEach(pi, &_pipelines) {
		if (pi->type == P_GRAPHICS)
			Sys_Free(pi->graphics.at);

		Re_BkDestroyPipeline(pi->pipeline);
	}

	Re_SavePipelineCache();
	Rt_TermArray(&_pipelines);
}
