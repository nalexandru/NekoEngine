#include <assert.h>

#define Handle __EngineHandle

#include <Engine/IO.h>
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
	desc.binaryArchives = @[ _cache ];
	
	struct Pipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	assert(p);
	
	p->type = PS_RENDER;
	
	for (uint32_t i = 0; i < gpDesc->shader->stageCount; ++i) {
		switch (gpDesc->shader->stages[i].stage) {
		case SS_VERTEX: [desc setVertexFunction: (id<MTLFunction>)gpDesc->shader->stages[i].module]; break;
		case SS_FRAGMENT: [desc setFragmentFunction: (id<MTLFunction>)gpDesc->shader->stages[i].module]; break;
		default: continue;
		}
	}
	
	desc.alphaToCoverageEnabled = gpDesc->flags & RE_ALPHA_TO_COVERAGE;
	desc.alphaToOneEnabled = gpDesc->flags & RE_ALPHA_TO_ONE;
	desc.rasterizationEnabled = true;
	
	for (uint32_t i = 0; i < gpDesc->attachmentCount; ++i) {
		desc.colorAttachments[i].pixelFormat = gpDesc->renderPassDesc->attachmentFormats[i];
		desc.colorAttachments[i].blendingEnabled = gpDesc->attachments[i].enableBlend;
	
		desc.colorAttachments[i].rgbBlendOperation = NeToMTLBlendOperation(gpDesc->attachments[i].colorOp);
		desc.colorAttachments[i].sourceRGBBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].srcColor);
		desc.colorAttachments[i].destinationRGBBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].dstColor);
	
		desc.colorAttachments[i].alphaBlendOperation = NeToMTLBlendOperation(gpDesc->attachments[i].alphaOp);
		desc.colorAttachments[i].sourceAlphaBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].srcAlpha);
		desc.colorAttachments[i].destinationAlphaBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].dstAlpha);
	}
	
	if (gpDesc->flags & RE_DEPTH_TEST) {
		desc.depthAttachmentPixelFormat = NeToMTLTextureFormat(gpDesc->depthFormat);
	
		MTLDepthStencilDescriptor *dssDesc = [[MTLDepthStencilDescriptor alloc] init];
		
		dssDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
		dssDesc.depthWriteEnabled = true;
		
		p->render.depthStencil = [dev newDepthStencilStateWithDescriptor: dssDesc];
		[dssDesc release];
	}
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];
	
	//[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
	[desc autorelease];
	
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
	for (uint32_t i = 0; i < cpDesc->shader->stageCount; ++i) {
		if (cpDesc->shader->stages[i].stage != SS_COMPUTE)
			continue;
		
		stageDesc = &cpDesc->shader->stages[i];
		break;
	}
	
	if (!stageDesc)
		return NULL;
	
	MTLComputePipelineDescriptor *desc = [[MTLComputePipelineDescriptor alloc] init];
	desc.binaryArchives = @[ _cache ];
	
	desc.computeFunction = (id<MTLFunction>)stageDesc->module;
	
	NSError *err;
	id<MTLComputePipelineState> pso =
		[dev newComputePipelineStateWithDescriptor: desc
										   options: MTLPipelineOptionNone
										reflection: nil
											 error: &err];
	
	[_cache addComputePipelineFunctionsWithDescriptor: desc error: nil];
	
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

	[desc autorelease];
	
	return NULL;
}

struct Pipeline *
MTL_RayTracingPipeline(id<MTLDevice> dev, struct ShaderBindingTable *sbt, uint32_t maxDepth)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
	
	desc.binaryArchives = @[ _cache ];
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];
	
	[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
	[desc autorelease];
	
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
	MTLBinaryArchiveDescriptor *desc = [[MTLBinaryArchiveDescriptor alloc] init];
	
	desc.url = [Darwin_appSupportURL URLByAppendingPathComponent: @"pipeline.cache"];
	
	if (![[NSFileManager defaultManager] fileExistsAtPath: [desc.url path]])
		desc.url = nil;
	
	NSError *err;
	_cache = [dev newBinaryArchiveWithDescriptor: desc error: &err];
	
	[desc autorelease];
}

void
MTL_SavePipelineCache(id<MTLDevice> dev)
{
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
