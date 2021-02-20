#include "VulkanDriver.h"

struct Pipeline *
Vk_GraphicsPipeline(struct RenderDevice * dev, struct Shader * sh, uint64_t flags, const struct BlendAttachmentDesc *atDesc, uint32_t atCount)
{
	return NULL;
}

struct Pipeline *
Vk_ComputePipeline(struct RenderDevice *dev, struct Shader *sh)
{
	return NULL;
}

struct Pipeline *
Vk_RayTracingPipeline(struct RenderDevice *dev, struct ShaderBindingTable *sbt)
{
	return NULL;
}

void
Vk_LoadPipelineCache(struct RenderDevice *dev)
{
	//
}

void
Vk_SavePipelineCache(struct RenderDevice *dev)
{
	//
}

