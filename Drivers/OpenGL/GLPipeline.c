#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>

#include "OpenGLDriver.h"

#define DKPMOD	L"OpenGLPipeline"

//static VkPipelineCache _cache;

static inline GLenum
_NeToGLCompareOp(uint64_t flags)
{
	switch (flags & RE_DEPTH_OP_BITS) {
	case RE_DEPTH_OP_LESS: return GL_LESS;
	case RE_DEPTH_OP_EQUAL: return GL_EQUAL;
	case RE_DEPTH_OP_LESS_EQUAL: return GL_LEQUAL;
	case RE_DEPTH_OP_GREATER: return GL_GREATER;
	case RE_DEPTH_OP_NOT_EQUAL: return GL_NOTEQUAL;
	case RE_DEPTH_OP_GREATER_EQUAL: return GL_GEQUAL;
	case RE_DEPTH_OP_ALWAYS: return GL_ALWAYS;
	}
	
	return GL_GEQUAL;
}

struct NePipeline *
GL_GraphicsPipeline(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(1, sizeof(*p), MH_RenderDriver);
	if (!p)
		return NULL;

	const uint64_t flags = desc->flags;

	p->graphics.discard = flags & RE_DISCARD;
	p->graphics.depthBias = flags & RE_DEPTH_BIAS;
	p->graphics.depthClamp = flags & RE_DEPTH_CLAMP;

	p->graphics.sampleShading = flags & RE_SAMPLE_SHADING;
	p->graphics.alphaToCoverage = flags & RE_ALPHA_TO_COVERAGE;
	p->graphics.alphaToOne = flags & RE_ALPHA_TO_ONE;

	p->graphics.depthTest = (flags & RE_DEPTH_TEST) == RE_DEPTH_TEST;
	p->graphics.depthWrite = (flags & RE_DEPTH_WRITE) == RE_DEPTH_WRITE;
	p->graphics.depthOp = _NeToGLCompareOp(flags & RE_DEPTH_OP_BITS);
	p->graphics.depthBounds = (flags & RE_DEPTH_BOUNDS) == RE_DEPTH_BOUNDS;
	p->graphics.stencilTest = false; // stencil not supported in v1 of the render API

	switch (flags & RE_TOPOLOGY_BITS) {
	case RE_TOPOLOGY_TRIANGLES: p->graphics.topology = GL_TRIANGLES; break;
	case RE_TOPOLOGY_POINTS: p->graphics.topology = GL_POINTS; break;
	case RE_TOPOLOGY_LINES: p->graphics.topology = GL_LINES; break;
	}

	switch (flags & RE_POLYGON_BITS) {
	case RE_POLYGON_FILL: p->graphics.polygonMode = GL_FILL; break;
	case RE_POLYGON_LINE: p->graphics.polygonMode = GL_LINE; break;
	case RE_POLYGON_POINT: p->graphics.polygonMode = GL_POINT; break;
	}

	switch (flags & RE_CULL_BITS) {
	case RE_CULL_BACK: p->graphics.cullMode = GL_BACK; break;
	case RE_CULL_FRONT: p->graphics.cullMode = GL_FRONT; break;
	case RE_CULL_NONE: p->graphics.cullMode = GL_NONE; break;
	case RE_CULL_FRONT_AND_BACK: p->graphics.cullMode = GL_FRONT_AND_BACK; break;
	}

	switch (flags & RE_FRONT_FACE_BITS) {
	case RE_FRONT_FACE_CCW: p->graphics.frontFace = GL_CCW; break;
	case RE_FRONT_FACE_CW: p->graphics.frontFace = GL_CW; break;
	}

	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_draw_buffers_blend.txt
		/*cbAttachments[i].blendEnable = desc->attachments[i].enableBlend;

		cbAttachments[i].srcColorBlendFactor = desc->attachments[i].srcColor;
		cbAttachments[i].dstColorBlendFactor = desc->attachments[i].dstColor;
		cbAttachments[i].colorBlendOp = desc->attachments[i].colorOp;

		cbAttachments[i].srcAlphaBlendFactor = desc->attachments[i].srcAlpha;
		cbAttachments[i].dstAlphaBlendFactor = desc->attachments[i].dstAlpha;
		cbAttachments[i].alphaBlendOp = desc->attachments[i].alphaOp;

		cbAttachments[i].colorWriteMask = desc->attachments[i].writeMask;*/
	}

	p->program = glCreateProgram();

	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i)
		glAttachShader(p->program, (GLuint)(uint64_t)desc->stageInfo->stages[i].module);

	glLinkProgram(p->program);

/*#ifdef _DEBUG
	if (desc->name) {
		Vkd_SetObjectName(dev->dev, p->pipeline, VK_OBJECT_TYPE_PIPELINE, desc->name);
		Vkd_SetObjectName(dev->dev, p->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc->name);
	}
#endif*/

	return p;
}

struct NePipeline *
GL_ComputePipeline(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	if (!p)
		return NULL;

	p->program = glCreateProgram();

	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		if (desc->stageInfo->stages[i].stage != SS_COMPUTE)
			continue;

		glAttachShader(p->program, (GLuint)(uint64_t)desc->stageInfo->stages[i].module);
	}

	glLinkProgram(p->program);

/*#ifdef _DEBUG
	if (desc->name) {
		Vkd_SetObjectName(dev->dev, p->pipeline, VK_OBJECT_TYPE_PIPELINE, desc->name);
		Vkd_SetObjectName(dev->dev, p->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc->name);
	}
#endif*/

	return p;
}

struct NePipeline *
GL_RayTracingPipeline(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth)
{
	return NULL;
}

void
GL_LoadPipelineCache(struct NeRenderDevice *dev)
{
}

void
GL_SavePipelineCache(struct NeRenderDevice *dev)
{
}

void
GL_DestroyPipeline(struct NeRenderDevice *dev, struct NePipeline *pipeline)
{
	Sys_Free(pipeline);
}
