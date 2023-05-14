#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>

#include "GLBackend.h"

static inline GLenum
NeToGLDepthFunc(uint64_t flags)
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

static GLuint CreateProgram(const struct NeShaderStageInfo *stageInfo);

struct NePipeline *
Re_BkGraphicsPipeline(const struct NeGraphicsPipelineDesc *desc)
{
	struct NePipeline *p = NULL;

	// attachment count sanity check
	if (desc->attachmentCount > sizeof(p->attachments) / sizeof(p->attachments[0]))
		return false;

	GLuint program = CreateProgram(desc->stageInfo);
	if (!program)
		return NULL;

	if (!(p = Sys_Alloc(1, sizeof(*p), MH_RenderBackend)))
		return NULL;

	p->type = PT_GRAPHICS;
	p->program = program;
	const uint64_t flags = desc->flags;

	// TODO: input assembler
	for (uint32_t i = 0; i < desc->vertexDesc.attributeCount; ++i) {
		const struct NeVertexBinding *b = &desc->vertexDesc.bindings[i];
		const struct NeVertexAttribute *at = &desc->vertexDesc.attributes[i];

		/*
		 * 		case VF_FLOAT: return GL_R;
		case VF_FLOAT2: return ;
		case VF_FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
		case VF_FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		default: return VK_FORMAT_R32_SFLOAT;
		 */

		//glEnableVertexAttribArray(at->location);
		//glVertexAttribPointer(at->location, at-)
		//glEnableVertexArrayAttrib()

		/*
		 * 				case BufferDataType::Int:
				case BufferDataType::Short:
				case BufferDataType::Byte:
				case BufferDataType::UnsignedInt:
				case BufferDataType::UnsignedShort:
				case BufferDataType::UnsignedByte:
				{
					GL_CHECK(glVertexAttribIPointer(attrib.index,
						attrib.size,
						GL_AttribTypes[(int)attrib.type],
						(GLsizei)attrib.stride,
						(GLsizei *)attrib.ptr));
				}
				break;
				default:
				{
					GL_CHECK(glVertexAttribPointer(attrib.index,
						attrib.size,
						GL_AttribTypes[(int)attrib.type],
						attrib.normalize ? GL_TRUE : GL_FALSE,
						(GLsizei)attrib.stride,
						(GLsizei *)attrib.ptr));
				}
				break;
		 *
		attribs[i].location = at->location;
		attribs[i].binding = at->binding;
		attribs[i].format = NeToVkVertexFormat(at->format);
		attribs[i].offset = at->offset;*/
	}

	switch (flags & RE_TOPOLOGY_BITS) {
		case RE_TOPOLOGY_TRIANGLES: p->ps.polygonMode = GL_TRIANGLES; break;
		case RE_TOPOLOGY_POINTS: p->ps.polygonMode = GL_POINTS; break;
		case RE_TOPOLOGY_LINES: p->ps.polygonMode = GL_LINES; break;
	}

	p->ps.rasterizerDiscard = (flags & RE_DISCARD) == RE_DISCARD;

	switch (flags & RE_CULL_BITS) {
		case RE_CULL_BACK: p->ps.cullMode = GL_BACK; break;
		case RE_CULL_FRONT: p->ps.cullMode = GL_FRONT; break;
		case RE_CULL_NONE: p->ps.cullMode = 0; break;
		case RE_CULL_FRONT_AND_BACK: p->ps.cullMode = GL_FRONT_AND_BACK; break;
	}

	switch (flags & RE_FRONT_FACE_BITS) {
		case RE_FRONT_FACE_CCW: p->ps.frontFace = GL_CCW; break;
		case RE_FRONT_FACE_CW: p->ps.frontFace = GL_CW; break;
	}

	p->ps.sampleShading = (flags & RE_SAMPLE_SHADING) == RE_SAMPLE_SHADING;
	p->ps.alphaToCoverage = (flags & RE_ALPHA_TO_COVERAGE) == RE_ALPHA_TO_COVERAGE;
	p->ps.alphaToOne = (flags & RE_ALPHA_TO_ONE) == RE_ALPHA_TO_ONE;
	p->ps.multisampling = (flags & RE_MULTISAMPLE) == RE_MULTISAMPLE;
	if (p->ps.multisampling) {
		switch (flags & RE_SAMPLES_BITS) {
			case RE_MS_2_SAMPLES: p->ps.samples = 2; break;
			case RE_MS_4_SAMPLES: p->ps.samples = 4; break;
			case RE_MS_8_SAMPLES: p->ps.samples = 8; break;
			case RE_MS_16_SAMPLES: p->ps.samples = 17; break;
		}
	} else {
		p->ps.samples = 1;
	}

	p->ps.depthTest = (flags & RE_DEPTH_TEST) == RE_DEPTH_TEST;
	p->ps.depthBias = (flags & RE_DEPTH_BIAS) == RE_DEPTH_BIAS;
	p->ps.depthClamp = (flags & RE_DEPTH_CLAMP) == RE_DEPTH_CLAMP;
	p->ps.depthWrite = (flags & RE_DEPTH_WRITE) == RE_DEPTH_WRITE;
	p->ps.depthFunc = NeToGLDepthFunc(flags & RE_DEPTH_OP_BITS);
	p->ps.depthBounds = (flags & RE_DEPTH_BOUNDS) == RE_DEPTH_BOUNDS;
	p->ps.minDepthBounds = 0.f;
	p->ps.maxDepthBounds = 1.;
	// stencil not supported in v1 of the render API

#ifdef _DEBUG
	/*if (desc->name) {
		Vkd_SetObjectName(Re_device->dev, p->pipeline, VK_OBJECT_TYPE_PIPELINE, desc->name);
		Vkd_SetObjectName(Re_device->dev, p->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc->name);
	}*/
#endif

	return p;
}

struct NePipeline *
Re_BkComputePipeline(const struct NeComputePipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderBackend);
	if (!p)
		return NULL;

	GLuint program = CreateProgram(desc->stageInfo);
	if (!program)
		return NULL;

	if (!(p = Sys_Alloc(1, sizeof(*p), MH_RenderBackend)))
		return NULL;

	p->type = PT_COMPUTE;
	p->program = program;

#ifdef _DEBUG
	/*if (desc->name) {
		Vkd_SetObjectName(Re_device->dev, p->pipeline, VK_OBJECT_TYPE_PIPELINE, desc->name);
		Vkd_SetObjectName(Re_device->dev, p->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc->name);
	}*/
#endif

	return p;
}

void
Re_BkDestroyPipeline(struct NePipeline *pipeline)
{
	Sys_Free(pipeline);
}

void
GLBk_ApplyPipelineState(const struct GLBkPipelineState *ps)
{
	struct GLBkPipelineState *dps = &Re_device->state;

	if (dps->frontFace != ps->frontFace) {
		GL_TRACE(glFrontFace(ps->frontFace));
		dps->frontFace = ps->frontFace;
	}

	if (dps->fillMode != ps->fillMode) {
		GL_TRACE(glFrontFace(ps->frontFace));
		dps->frontFace = ps->frontFace;
	}

	if (dps->cullMode != ps->cullMode) {
		if (ps->cullMode == 0) {
			GL_TRACE(glDisable(GL_CULL_FACE));
		} else {
			GL_TRACE(glEnable(GL_CULL_FACE));
			GL_TRACE(glCullFace(ps->cullMode));
		}
		dps->cullMode = ps->cullMode;
	}

	if (dps->rasterizerDiscard != ps->rasterizerDiscard) {
		if (ps->rasterizerDiscard) {
			GL_TRACE(glEnable(GL_RASTERIZER_DISCARD_EXT));
		} else {
			GL_TRACE(glDisable(GL_RASTERIZER_DISCARD_EXT));
		}
		dps->rasterizerDiscard = ps->rasterizerDiscard;
	}

	if (dps->depthTest != ps->depthTest) {
		if (ps->depthTest) {
			GL_TRACE(glEnable(GL_DEPTH_TEST));
		} else {
			GL_TRACE(glDisable(GL_DEPTH_TEST));
		}
		dps->depthTest = ps->depthTest;
	}

	if (dps->depthWrite != ps->depthWrite) {
		GL_TRACE(glDepthMask(dps->depthWrite));
		dps->depthWrite = ps->depthWrite;
	}

	if (dps->depthBounds != ps->depthBounds) {
		if (ps->depthBounds) {
			GL_TRACE(glEnable(GL_DEPTH_BOUNDS_TEST_EXT));
			GL_TRACE(glDepthBoundsEXT(ps->minDepthBounds, ps->maxDepthBounds));
		} else {
			GL_TRACE(glDisable(GL_DEPTH_BOUNDS_TEST_EXT));
		}

		dps->depthBounds = ps->depthBounds;
		dps->minDepthBounds = ps->minDepthBounds;
		dps->maxDepthBounds = ps->maxDepthBounds;
	}

	if (dps->depthClamp != ps->depthClamp) {
		if (ps->depthClamp) {
			GL_TRACE(glEnable(GL_DEPTH_CLAMP));
		} else {
			GL_TRACE(glDisable(GL_DEPTH_CLAMP));
		}
		dps->depthClamp = ps->depthClamp;
	}

	if (dps->depthBias != ps->depthBias) {
		if (ps->depthClamp) {
			GL_TRACE(glEnable(GL_DEPTH_BIAS));
		} else {
			GL_TRACE(glDisable(GL_DEPTH_BIAS));
		}
		dps->depthBias = ps->depthBias;
	}

	if (dps->depthFunc != ps->depthFunc) {
		GL_TRACE(glDepthFunc(ps->depthFunc));
		dps->depthFunc = ps->depthFunc;
	}

	//

	//bool multisampling, sampleShading, alphaToCoverage, alphaToOne;
	//uint32_t samples;
}

static GLuint
CreateProgram(const struct NeShaderStageInfo *stageInfo)
{
	GLuint p = GL_TRACE(glCreateProgram());

	struct NeArray pipelineUniforms;
	for (uint32_t i = 0; i < stageInfo->stageCount; ++i) {
		const struct GLBkShaderModule *mod = stageInfo->stages[i].module;
		GL_TRACE(glAttachShader(p, mod->id));
	}

	GL_TRACE(glLinkProgram(p));
	GLint rc;
	GL_TRACE(glGetProgramiv(p, GL_LINK_STATUS, &rc));

	if (rc)
		return p;

	for (uint32_t i = 0; i < stageInfo->stageCount; ++i) {
		const struct GLBkShaderModule *mod = stageInfo->stages[i].module;

		struct GLBkShaderUniform *u;
		Rt_ArrayForEach(u, &mod->uniforms) {
			struct GLBkPipelineUniform *pu = Rt_ArrayAllocate(&pipelineUniforms);
			if (!pu) {
				// FIXME: error
			}

			pu->type = u->type;
			pu->size = u->size;

			if (pu->type == UT_NONE)
				continue;

			pu->location = GL_TRACE(glGetUniformLocation(p, u->name));
		}
	}

	GL_TRACE(glDeleteProgram(p));
	return 0;
}

/* NekoEngine
 *
 * GLPipeline.c
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
