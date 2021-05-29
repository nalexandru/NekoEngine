#include "MTLDriver.h"

MTLTextureDescriptor *
MTL_TextureDescriptor(id<MTLDevice> dev, const struct TextureCreateInfo *tci)
{
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
	
	return desc;
}

struct Texture *
MTL_CreateTexture(id<MTLDevice> dev, const struct TextureCreateInfo *tci, uint16_t location)
{
	MTLTextureDescriptor *desc = MTL_TextureDescriptor(dev, tci);
	if (!desc)
		return NULL;
	
	struct Texture *tex = Sys_Alloc(sizeof(*tex), 1, MH_RenderDriver);
	if (!tex) {
		[desc release];
		return NULL;
	}
	
	memcpy(&tex->desc, &tci->desc, sizeof(tex->desc));
	tex->tex = [dev newTextureWithDescriptor: desc];
	
	[desc release];
	
	MTL_SetTexture(location, tex->tex);
	
	if (tci->data)
		MTL_UploadTexture(dev, tex->tex, tci->data, tci->dataSize, tci->desc.width, tci->desc.height, tci->desc.depth);
	
	return tex;
}

void
MTL_UploadTexture(id<MTLDevice> dev, id<MTLTexture> tex, const void *data, NSUInteger size, uint32_t width, uint32_t height, uint32_t depth)
{
	id<MTLBuffer> staging = [dev newBufferWithBytes: data length: size options: MTLResourceStorageModeShared];
		
	id<MTLCommandQueue> queue = [dev newCommandQueue];
	id<MTLCommandBuffer> cmdBuffer = [queue commandBuffer];
	id<MTLBlitCommandEncoder> encoder = [cmdBuffer blitCommandEncoder];
		
	[encoder copyFromBuffer: staging
			   sourceOffset: 0
		  sourceBytesPerRow: width * 4 * sizeof(uint8_t)
		sourceBytesPerImage: 0
				 sourceSize: MTLSizeMake(width, height, depth)
				  toTexture: tex
		   destinationSlice: 0
		   destinationLevel: 0
		  destinationOrigin: MTLOriginMake(0, 0, 0)];
	[encoder endEncoding];
		
	[cmdBuffer commit];
	[cmdBuffer waitUntilCompleted];
		
#if TARGET_OS_OSX
	[encoder autorelease];
	[cmdBuffer autorelease];
	[queue autorelease];
	[staging autorelease];
#endif
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
	Sys_Free(tex);
}
