#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/Texture.h>

#undef Handle

#include "MTLDriver.h"

struct Texture *
MTL_CreateTextureInternal(id<MTLDevice> dev, const struct TextureCreateInfo *tci, bool transient, uint16_t location)
{
	struct Texture *tex = malloc(sizeof(*tex));
	MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
	
	desc.pixelFormat = NeToMTLTextureFormat(tci->desc.format);
	if (desc.pixelFormat == MTLPixelFormatInvalid) {
		[desc autorelease];
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
	
	if (transient)
		desc.resourceOptions |= MTLResourceStorageModeMemoryless;
	
	memcpy(&tex->desc, &tci->desc, sizeof(tex->desc));
	tex->tex = [dev newTextureWithDescriptor: desc];
	
	[desc release];
	
	MTL_SetTexture(location, tex->tex);
	
	if (tci->data) {
		id<MTLBuffer> staging = [dev newBufferWithBytes: tci->data length: tci->dataSize options: MTLResourceStorageModeShared];
		
		id<MTLCommandQueue> queue = [dev newCommandQueue];
		id<MTLCommandBuffer> cmdBuffer = [queue commandBuffer];
		id<MTLBlitCommandEncoder> encoder = [cmdBuffer blitCommandEncoder];
		
		[encoder copyFromBuffer: staging
				   sourceOffset: 0
			  sourceBytesPerRow: tci->desc.width * 4 * sizeof(uint8_t)
			sourceBytesPerImage: 0
					 sourceSize: MTLSizeMake(tci->desc.width, tci->desc.height, tci->desc.depth)
					  toTexture: tex->tex
			   destinationSlice: 0
			   destinationLevel: 0
			  destinationOrigin: MTLOriginMake(0, 0, 0)];
		[encoder endEncoding];
		
		[cmdBuffer commit];
		[cmdBuffer waitUntilCompleted];
		
		[encoder autorelease];
		[cmdBuffer autorelease];
		[queue autorelease];
		[staging autorelease];
	}
	
	return tex;
}

struct Texture *
MTL_CreateTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci, uint16_t location)
{
	return MTL_CreateTextureInternal(dev, tci, false, location);
}


enum TextureLayout
MTL_TextureLayout(const struct Texture *tex)
{
	return tex->layout;
}

void
MTL_DestroyTexture(id<MTLDevice> dev, struct Texture *tex)
{
	MTL_RemoveTexture(tex->tex);
	[tex->tex autorelease];
	free(tex);
}
