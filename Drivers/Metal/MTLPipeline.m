#define Handle __EngineHandle

#include <Engine/IO.h>
#include <Engine/Config.h>
#include <System/Log.h>

#undef Handle

#include "MTLDriver.h"

#define MPMOD	L"MetalPipeline"

extern NSURL *Darwin_appSupportURL;

static id<MTLBinaryArchive> _cache;

struct Pipeline *
MTL_GraphicsPipeline(id<MTLDevice> dev, const struct GraphicsPipelineDesc *gpDesc)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
	if (_cache)
		desc.binaryArchives = @[ _cache ];
	
	struct Pipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	if (!p) {
		[desc release];
		return NULL;
	}
	
	p->type = PS_RENDER;
	
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
		
		p->render.depthStencil = [dev newDepthStencilStateWithDescriptor: dssDesc];
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
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];

	if (_cache)
		[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
	[desc release];
	
	if (!pso) {
		Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create graphics pipeline: %hs",
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

struct Pipeline *
MTL_ComputePipeline(id<MTLDevice> dev, const struct ComputePipelineDesc *cpDesc)
{
	struct ShaderStageDesc *stageDesc = NULL;
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
		[dev newComputePipelineStateWithDescriptor: desc
										   options: MTLPipelineOptionNone
										reflection: nil
											 error: &err];

	if (_cache)
		[_cache addComputePipelineFunctionsWithDescriptor: desc error: nil];
	[desc release];
	
	if (pso) {
		struct Pipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
		p->type = PS_COMPUTE;
		p->compute.state = pso;
		p->compute.threadsPerThreadgroup = MTLSizeMake(cpDesc->threadsPerThreadgroup.x,
														cpDesc->threadsPerThreadgroup.y,
														cpDesc->threadsPerThreadgroup.z);
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create compute pipeline: %hs",
				 [[err localizedDescription] UTF8String]);
	
	return NULL;
}

struct Pipeline *
MTL_RayTracingPipeline(id<MTLDevice> dev, struct ShaderBindingTable *sbt, uint32_t maxDepth)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];

	if (_cache)
		desc.binaryArchives = @[ _cache ];
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];

	if (_cache)
		[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
	[desc release];
	
	if (pso) {
		struct Pipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
		p->type = PS_RAY_TRACING;
		p->render.state = pso;
		
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create ray tracing pipeline: %hs",
				 [[err localizedDescription] UTF8String]);
	
	return NULL;
}

void
MTL_LoadPipelineCache(id<MTLDevice> dev)
{
	if (!E_GetCVarBln(L"MetalDrv_EnableBinaryArchive", false)->bln)
		return;

	MTLBinaryArchiveDescriptor *desc = [[MTLBinaryArchiveDescriptor alloc] init];
	
	desc.url = [Darwin_appSupportURL URLByAppendingPathComponent: @"pipeline.cache"];
	
	if (![[NSFileManager defaultManager] fileExistsAtPath: [desc.url path]])
		desc.url = nil;
	
	NSError *err;
	_cache = [dev newBinaryArchiveWithDescriptor: desc error: &err];
	
	[desc release];
}

void
MTL_SavePipelineCache(id<MTLDevice> dev)
{
	if (!E_GetCVarBln(L"MetalDrv_EnableBinaryArchive", false)->bln)
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
MTL_DestroyPipeline(id<MTLDevice> dev, struct Pipeline *p)
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
