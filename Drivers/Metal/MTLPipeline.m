#define Handle __EngineHandle

#include <Engine/IO.h>
#include <System/Log.h>
#include <Render/Device.h>
#include <Render/Pipeline.h>

#undef Handle

#include "MTLDriver.h"

#define MPMOD	L"MetalPipeline"

extern NSURL *Darwin_appSupportURL;

static id<MTLBinaryArchive> _cache;

struct PipelineLayout *
MTL_CreatePipelineLayout(id<MTLDevice> dev, const struct PipelineLayoutDesc *desc)
{
	struct PipelineLayout *pl = calloc(1, sizeof(*pl));
	if (!pl)
		return NULL;
	
	pl->setCount = desc->setLayoutCount;
	pl->sets = calloc(desc->setLayoutCount, sizeof(*pl->sets));
	
	uint32_t firstBuffer = 0, firstTexture = 0, firstSampler = 0;
	for (uint32_t i = 0; i < desc->setLayoutCount; ++i) {
		pl->sets[i].firstBuffer = firstBuffer;
		pl->sets[i].firstTexture = firstTexture;
		pl->sets[i].firstSampler = firstSampler;
		
		const struct DescriptorSetLayoutDesc setDesc = desc->setLayouts[i]->desc;
		
		for (uint32_t j = 0; j < setDesc.bindingCount; ++j) {
			if (setDesc.bindings[j].type == DT_TEXTURE)
				firstTexture += setDesc.bindings[j].count;
			else if (setDesc.bindings[j].type == DT_SAMPLER)
				firstSampler += setDesc.bindings[j].count;
			else
				firstBuffer += setDesc.bindings[j].count;
		}
	}
	
	pl->pushConstantIndex = firstBuffer;
	
	return pl;
}

void
MTL_DestroyPipelineLayout(id<MTLDevice> dev, struct PipelineLayout *layout)
{
	free(layout->sets);
	free(layout);
}

struct Pipeline *
MTL_GraphicsPipeline(id<MTLDevice> dev, const struct GraphicsPipelineDesc *gpDesc)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
	desc.binaryArchives = @[ _cache ];
	
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
		desc.colorAttachments[i].pixelFormat = gpDesc->renderPass->attachmentFormats[i];
		desc.colorAttachments[i].blendingEnabled = gpDesc->attachments[i].enableBlend;
	
		desc.colorAttachments[i].rgbBlendOperation = NeToMTLBlendOperation(gpDesc->attachments[i].colorOp);
		desc.colorAttachments[i].sourceRGBBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].srcColor);
		desc.colorAttachments[i].destinationRGBBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].dstColor);
	
		desc.colorAttachments[i].alphaBlendOperation = NeToMTLBlendOperation(gpDesc->attachments[i].alphaOp);
		desc.colorAttachments[i].sourceAlphaBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].srcAlpha);
		desc.colorAttachments[i].destinationAlphaBlendFactor = NeToMTLBlendFactor(gpDesc->attachments[i].dstAlpha);
	}
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];
	
	//[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
	[desc release];
	
	if (!pso) {
		Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create graphics pipeline: %hs",
					 [[err localizedDescription] UTF8String]);
		
		return NULL;
	}
		
	struct Pipeline *p = malloc(sizeof(*p));
	p->type = PS_RENDER;
		
	switch (gpDesc->flags & RE_TOPOLOGY_BITS) {
	case RE_TOPOLOGY_TRIANGLES: p->render.primitiveType = MTLPrimitiveTypeTriangle; break;
	case RE_TOPOLOGY_LINES: p->render.primitiveType = MTLPrimitiveTypeLine; break;
	case RE_TOPOLOGY_POINTS: p->render.primitiveType = MTLPrimitiveTypePoint; break;
	}
	
	p->render.state = pso;
		
	return p;
}

struct Pipeline *
MTL_ComputePipeline(id<MTLDevice> dev, struct Shader *sh)
{
	struct ShaderStageDesc *stageDesc = NULL;
	for (uint32_t i = 0; i < sh->stageCount; ++i) {
		if (sh->stages[i].stage != SS_COMPUTE)
			continue;
		
		stageDesc = &sh->stages[i];
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
		struct Pipeline *p = malloc(sizeof(*p));
		p->type = PS_COMPUTE;
		p->computeState = pso;
		
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create compute pipeline: %hs",
				 [[err localizedDescription] UTF8String]);

	[desc release];
	
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
	[desc release];
	
	if (pso) {
		struct Pipeline *p = malloc(sizeof(*p));
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
	
	[desc release];
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
