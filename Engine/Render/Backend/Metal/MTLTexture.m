#include "MTLBackend.h"

MTLTextureDescriptor *
MTL_TextureDescriptor(id<MTLDevice> dev, const struct NeTextureDesc *tDesc)
{
	MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
	
	desc.pixelFormat = NeToMTLTextureFormat(tDesc->format);
	if (desc.pixelFormat == MTLPixelFormatInvalid) {
		[desc release];
		return NULL;
	}
	
	switch (tDesc->type) {
	case TT_2D: desc.textureType = MTLTextureType2D; break;
	case TT_3D: desc.textureType = MTLTextureType3D; break;
	case TT_Cube: desc.textureType = MTLTextureTypeCube; break;
	case TT_2D_Multisample: desc.textureType = MTLTextureType2DMultisample; break;
	};
	
	desc.usage = 0;
	if (tDesc->usage & TU_SAMPLED) desc.usage |= MTLTextureUsageShaderRead;
	if (tDesc->usage & TU_STORAGE) desc.usage |= (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite);
	if (tDesc->usage & TU_COLOR_ATTACHMENT) desc.usage |= MTLTextureUsageRenderTarget;
	if (tDesc->usage & TU_DEPTH_STENCIL_ATTACHMENT) desc.usage |= MTLTextureUsageRenderTarget;
	if (tDesc->usage & TU_INPUT_ATTACHMENT) desc.usage |= MTLTextureUsageShaderRead;
	
	desc.resourceOptions = MTL_GPUMemoryTypetoResourceOptions([dev hasUnifiedMemory], tDesc->memoryType);
	desc.allowGPUOptimizedContents = tDesc->gpuOptimalTiling;
	desc.width = tDesc->width;
	desc.height = tDesc->height;
	desc.depth = tDesc->depth;
	desc.arrayLength = desc.textureType == MTLTextureTypeCube ? 1 : tDesc->arrayLayers;
	desc.mipmapLevelCount = tDesc->mipLevels;
	
	return desc;
}

struct NeTexture *
Re_BkCreateTexture(const struct NeTextureDesc *tDesc, uint16_t location)
{
	MTLTextureDescriptor *desc = MTL_TextureDescriptor(MTL_device, tDesc);
	if (!desc)
		return NULL;
	
	struct NeTexture *tex = Sys_Alloc(sizeof(*tex), 1, MH_RenderDriver);
	if (!tex) {
		[desc release];
		return NULL;
	}
	
	tex->tex = MTLDrv_CreateTexture(MTL_device, desc);
	[desc release];
	if (!tex->tex) {
		Sys_Free(tex);
		return NULL;
	}
	
	MTL_SetTexture(location, tex->tex);
	
	return tex;
}

enum NeTextureLayout
Re_BkTextureLayout(const struct NeTexture *tex)
{
	return tex->layout;
}

void
Re_BkDestroyTexture(struct NeTexture *tex)
{
	MTL_RemoveTexture(tex->tex);
	[tex->tex release];
	Sys_Free(tex);
}
