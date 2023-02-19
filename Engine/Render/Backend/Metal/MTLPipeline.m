#include <Engine/IO.h>
#include <Engine/Config.h>
#include <System/Log.h>

#include "MTLBackend.h"

#define MPMOD	"MetalPipeline"

extern NSURL *Darwin_appSupportURL;

static id<MTLBinaryArchive> _cache;

struct NePipeline *
Re_BkGraphicsPipeline(const struct NeGraphicsPipelineDesc *gpDesc)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
	if (_cache)
		desc.binaryArchives = @[ _cache ];
	
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	if (!p) {
		[desc release];
		return NULL;
	}
	
	p->type = PS_RENDER;

	MTLVertexDescriptor *vtxDesc = [MTLVertexDescriptor vertexDescriptor];

	for (uint32_t i = 0; i < gpDesc->vertexDesc.attributeCount; ++i) {
		const struct NeVertexAttribute *at = &gpDesc->vertexDesc.attributes[i];

		vtxDesc.attributes[at->location].format = NeToMTLVertexFormat(at->format);
		vtxDesc.attributes[at->location].offset = at->offset;
		vtxDesc.attributes[at->location].bufferIndex = at->binding + 2;
	}

	for (uint32_t i = 0; i < gpDesc->vertexDesc.bindingCount; ++i) {
		const struct NeVertexBinding *b = &gpDesc->vertexDesc.bindings[i];

		vtxDesc.layouts[b->binding + 2].stride = b->stride;
		vtxDesc.layouts[b->binding + 2].stepFunction = (MTLVertexStepFunction)(b->inputRate + 1);
	}

	[desc setVertexDescriptor: vtxDesc];
	
	for (uint32_t i = 0; i < gpDesc->stageInfo->stageCount; ++i) {
		switch (gpDesc->stageInfo->stages[i].stage) {
		case SS_VERTEX: [desc setVertexFunction: (id<MTLFunction>)gpDesc->stageInfo->stages[i].module]; break;
		case SS_FRAGMENT: [desc setFragmentFunction: (id<MTLFunction>)gpDesc->stageInfo->stages[i].module]; break;
		default: continue;
		}
	}
	
	desc.alphaToCoverageEnabled = gpDesc->flags & RE_ALPHA_TO_COVERAGE;
	desc.alphaToOneEnabled = gpDesc->flags & RE_ALPHA_TO_ONE;
	desc.rasterizationEnabled = true;
	
	for (uint32_t i = 0; i < gpDesc->renderPassDesc->colorAttachments; ++i) {
		desc.colorAttachments[i].pixelFormat = gpDesc->renderPassDesc->attachmentFormats[i];
		desc.colorAttachments[i].blendingEnabled = gpDesc->attachments[i].enableBlend;
	
		desc.colorAttachments[i].rgbBlendOperation = NeToMTLBlendOperation(gpDesc->attachments[i].colorOp);
		desc.colorAttachments[i].sourceRGBBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].srcColor);
		desc.colorAttachments[i].destinationRGBBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].dstColor);
	
		desc.colorAttachments[i].alphaBlendOperation = NeToMTLBlendOperation(gpDesc->attachments[i].alphaOp);
		desc.colorAttachments[i].sourceAlphaBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].srcAlpha);
		desc.colorAttachments[i].destinationAlphaBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].dstAlpha);
	}

	bool depth = false;
	if (gpDesc->flags & RE_DEPTH_TEST) {
		desc.depthAttachmentPixelFormat = NeToMTLTextureFormat(gpDesc->depthFormat);
	
		MTLDepthStencilDescriptor *dssDesc = [[MTLDepthStencilDescriptor alloc] init];
		
		switch (gpDesc->flags & RE_DEPTH_OP_BITS) {
		case RE_DEPTH_OP_LESS: dssDesc.depthCompareFunction = MTLCompareFunctionLess; break;
		case RE_DEPTH_OP_EQUAL: dssDesc.depthCompareFunction = MTLCompareFunctionEqual; break;
		case RE_DEPTH_OP_LESS_EQUAL: dssDesc.depthCompareFunction = MTLCompareFunctionLessEqual; break;
		case RE_DEPTH_OP_GREATER: dssDesc.depthCompareFunction = MTLCompareFunctionGreater; break;
		case RE_DEPTH_OP_NOT_EQUAL: dssDesc.depthCompareFunction = MTLCompareFunctionNotEqual; break;
		case RE_DEPTH_OP_GREATER_EQUAL: dssDesc.depthCompareFunction = MTLCompareFunctionGreaterEqual; break;
		case RE_DEPTH_OP_ALWAYS: dssDesc.depthCompareFunction = MTLCompareFunctionAlways; break;
		}
		dssDesc.depthWriteEnabled = (gpDesc->flags & RE_DEPTH_WRITE) == RE_DEPTH_WRITE;
		
		p->render.depthStencil = [MTL_device newDepthStencilStateWithDescriptor: dssDesc];
		[dssDesc release];

		depth = true;
	}

	for (uint32_t i = 0; i < gpDesc->renderPassDesc->inputAttachments; ++i) {
		const uint32_t idx = gpDesc->renderPassDesc->colorAttachments + i;
		desc.colorAttachments[idx].pixelFormat = gpDesc->renderPassDesc->attachmentFormats[idx];
		desc.colorAttachments[idx].blendingEnabled = false;
	}

	desc.vertexBuffers[0].mutability = MTLMutabilityImmutable;
	desc.vertexBuffers[1].mutability = MTLMutabilityImmutable;

	desc.fragmentBuffers[0].mutability = MTLMutabilityImmutable;
	desc.fragmentBuffers[1].mutability = MTLMutabilityImmutable;

	NSError *err;
	id<MTLRenderPipelineState> pso = [MTL_device newRenderPipelineStateWithDescriptor: desc error: &err];

	if (_cache)
		[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];

	[desc release];
	
	if (!pso) {
		Sys_LogEntry(MPMOD, LOG_CRITICAL, "Failed to create graphics pipeline: %s",
					 [[err localizedDescription] UTF8String]);
		
		return NULL;
	}
		
	switch (gpDesc->flags & RE_TOPOLOGY_BITS) {
	case RE_TOPOLOGY_TRIANGLES: p->render.primitiveType = MTLPrimitiveTypeTriangle; break;
	case RE_TOPOLOGY_LINES: p->render.primitiveType = MTLPrimitiveTypeLine; break;
	case RE_TOPOLOGY_POINTS: p->render.primitiveType = MTLPrimitiveTypePoint; break;
	}
	
	p->render.state = pso;
		
	return p;
}

struct NePipeline *
Re_BkComputePipeline(const struct NeComputePipelineDesc *cpDesc)
{
	struct NeShaderStageDesc *stageDesc = NULL;
	for (uint32_t i = 0; i < cpDesc->stageInfo->stageCount; ++i) {
		if (cpDesc->stageInfo->stages[i].stage != SS_COMPUTE)
			continue;
		
		stageDesc = &cpDesc->stageInfo->stages[i];
		break;
	}
	
	if (!stageDesc)
		return NULL;
	
	MTLComputePipelineDescriptor *desc = [[MTLComputePipelineDescriptor alloc] init];

	if (_cache)
		desc.binaryArchives = @[ _cache ];
	
	desc.computeFunction = (id<MTLFunction>)stageDesc->module;
	
	NSError *err;
	id<MTLComputePipelineState> pso =
		[MTL_device newComputePipelineStateWithDescriptor: desc
												  options: MTLPipelineOptionNone
											   reflection: nil
													error: &err];

	if (_cache)
		[_cache addComputePipelineFunctionsWithDescriptor: desc error: nil];
	[desc release];
	
	if (pso) {
		struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
		p->type = PS_COMPUTE;
		p->compute.state = pso;
		p->compute.threadsPerThreadgroup = MTLSizeMake(cpDesc->threadsPerThreadgroup.x,
														cpDesc->threadsPerThreadgroup.y,
														cpDesc->threadsPerThreadgroup.z);
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, "Failed to create compute pipeline: %hs",
				 [[err localizedDescription] UTF8String]);
	
	return NULL;
}

struct NePipeline *
Re_BkRayTracingPipeline(const struct NeRayTracingPipelineDesc *desc)
{
	MTLRenderPipelineDescriptor *pDesc = [[MTLRenderPipelineDescriptor alloc] init];

	if (_cache)
		pDesc.binaryArchives = @[ _cache ];
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [MTL_device newRenderPipelineStateWithDescriptor: pDesc error: &err];

	if (_cache)
		[_cache addRenderPipelineFunctionsWithDescriptor: pDesc error: nil];
	[pDesc release];
	
	if (pso) {
		struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
		p->type = PS_RAY_TRACING;
		p->render.state = pso;
		
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, "Failed to create ray tracing pipeline: %hs",
				 [[err localizedDescription] UTF8String]);
	
	return NULL;
}

void
Re_LoadPipelineCache(void)
{
	if (!E_GetCVarBln("MetalBackend_EnableBinaryArchive", false)->bln)
		return;

	MTLBinaryArchiveDescriptor *desc = [[MTLBinaryArchiveDescriptor alloc] init];
	
	desc.url = [Darwin_appSupportURL URLByAppendingPathComponent: @"pipeline.cache"];
	
	if (![[NSFileManager defaultManager] fileExistsAtPath: [desc.url path]])
		desc.url = nil;
	
	NSError *err;
	_cache = [MTL_device newBinaryArchiveWithDescriptor: desc error: &err];
	
	[desc release];
}

void
Re_SavePipelineCache(void)
{
	if (!E_GetCVarBln("MetalBackend_EnableBinaryArchive", false)->bln)
		return;

	MTLBinaryArchiveDescriptor *desc = [[MTLBinaryArchiveDescriptor alloc] init];
	
	NSURL *cacheUrl = [Darwin_appSupportURL URLByAppendingPathComponent: @"pipeline.cache"];
	
	NSError *err;
	[_cache serializeToURL: cacheUrl error: &err];
	
	[desc release];
	[_cache release];
	[cacheUrl release];
}

void
Re_BkDestroyPipeline(struct NePipeline *p)
{
	switch (p->type) {
	case PS_RENDER:
		[p->render.state release];
		if (p->render.depthStencil)
			[p->render.depthStencil release];
	break;
	case PS_RAY_TRACING: [p->render.state release]; break;
	case PS_COMPUTE: [p->compute.state release]; break;
	}
	Sys_Free(p);
}

/* NekoEngine
 *
 * MTLPipeline.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
