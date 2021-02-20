#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/Texture.h>

#undef Handle

#include "MTLDriver.h"

struct Texture *
MTL_CreateTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci)
{
	struct Texture *tex = malloc(sizeof(*tex));
	MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
	
	desc.pixelFormat = NeToMTLTextureFormat(tci->desc.format);
	if (desc.pixelFormat == MTLPixelFormatInvalid) {
		[desc release];
		return NULL;
	}
	
	switch (tci->desc.type) {
	case TT_2D: desc.textureType = MTLTextureType2D; break;
	case TT_3D: desc.textureType = MTLTextureType3D; break;
	case TT_Cube: desc.textureType = MTLTextureTypeCube; break;
	case TT_2D_Multisample: desc.textureType = MTLTextureType2DMultisample; break;
	};
	
	desc.usage = 0;
	if (tci->desc.usage & TU_SAMPLED) desc.usage |= MTLTextureUsageShaderRead;
	if (tci->desc.usage & TU_STORAGE) desc.usage |= (MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite);
	if (tci->desc.usage & TU_COLOR_ATTACHMENT) desc.usage |= MTLTextureUsageRenderTarget;
	if (tci->desc.usage & TU_DEPTH_STENCIL_ATTACHMENT) desc.usage |= MTLTextureUsageRenderTarget;
	if (tci->desc.usage & TU_INPUT_ATTACHMENT) desc.usage |= MTLTextureUsageShaderRead;
	
	desc.resourceOptions = MTL_GPUMemoryTypetoResourceOptions([dev hasUnifiedMemory], tci->desc.memoryType);
	desc.allowGPUOptimizedContents = tci->desc.gpuOptimalTiling;
	desc.width = tci->desc.width;
	desc.height = tci->desc.height;
	desc.depth = tci->desc.depth;
	desc.arrayLength = tci->desc.arrayLayers;
	desc.mipmapLevelCount = tci->desc.mipLevels;
	
	memcpy(&tex->desc, &tci->desc, sizeof(tex->desc));
	tex->tex = [dev newTextureWithDescriptor: desc];
	
	[desc release];
	
	return tex;
}

const struct TextureDesc *
MTL_TextureDesc(const struct Texture *tex)
{
	return &tex->desc;
}

void
MTL_DestroyTexture(id<MTLDevice> dev, struct Texture *tex)
{
	[tex->tex release];
	free(tex);
}
