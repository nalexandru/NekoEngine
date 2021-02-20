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
	
	uint32_t firstBuffer = 0, firstTexture = 0;
	for (uint32_t i = 0; i < desc->setLayoutCount; ++i) {
		pl->sets[i].firstBuffer = firstBuffer;
		pl->sets[i].firstTexture = firstTexture;
		
		const struct DescriptorSetLayoutDesc setDesc = desc->setLayouts[i];
		
		for (uint32_t j = 0; j < setDesc.bindingCount; ++j) {
			if (setDesc.bindings[j].type == DT_TEXTURE)
				firstTexture += setDesc.bindings[j].count;
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
MTL_GraphicsPipeline(id<MTLDevice> dev, struct Shader *sh, uint64_t flags, struct BlendAttachmentDesc *at, uint32_t atCount)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];
	
	[desc release];
	
	if (pso) {
		struct Pipeline *p = malloc(sizeof(*p));
		p->type = PS_RENDER;
		
		// FIXME
		p->render.primitiveType = MTLPrimitiveTypeTriangle;
		p->render.state = pso;
		
		[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
		
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create graphics pipeline: %hs",
				 [[err localizedDescription] UTF8String]);
	
	return NULL;
}

struct Pipeline *
MTL_ComputePipeline(id<MTLDevice> dev, struct Shader *sh)
{
	MTLComputePipelineDescriptor *desc = [[MTLComputePipelineDescriptor alloc] init];
	
	desc.computeFunction = sh->function;
	desc.binaryArchives = @[ _cache ];
	
	NSError *err;
	id<MTLComputePipelineState> pso =
		[dev newComputePipelineStateWithDescriptor: desc
										   options: MTLPipelineOptionNone
										reflection: nil
											 error: &err];
	[desc release];
	
	if (pso) {
		struct Pipeline *p = malloc(sizeof(*p));
		p->type = PS_COMPUTE;
		p->computeState = pso;
		
		[_cache addComputePipelineFunctionsWithDescriptor: desc error: nil];
		
		return p;
	}
	
	Sys_LogEntry(MPMOD, LOG_CRITICAL, L"Failed to create compute pipeline: %hs",
				 [[err localizedDescription] UTF8String]);

	return NULL;
}

struct Pipeline *
MTL_RayTracingPipeline(id<MTLDevice> dev, struct ShaderBindingTable *sbt)
{
	MTLRenderPipelineDescriptor *desc = [[MTLRenderPipelineDescriptor alloc] init];
	
	desc.binaryArchives = @[ _cache ];
	
	NSError *err;
	id<MTLRenderPipelineState> pso = [dev newRenderPipelineStateWithDescriptor: desc error: &err];
	
	[desc release];
	
	if (pso) {
		struct Pipeline *p = malloc(sizeof(*p));
		p->type = PS_RENDER;
		p->render.state = pso;
		
		[_cache addRenderPipelineFunctionsWithDescriptor: desc error: nil];
			
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
