#ifndef _NE_RENDER_DRIVER_DRIVER_H_
#define _NE_RENDER_DRIVER_DRIVER_H_

#include <Render/Types.h>

#define NE_RENDER_DRIVER_ID		0xB16B00B5
#define NE_RENDER_DRIVER_API	27

#define RE_API_NULL			0
#define RE_API_VULKAN		1
#define RE_API_METAL		2
#define RE_API_DIRECT3D12	3
#define RE_API_OPENGL		4

struct NeRenderDriver
{
	uint32_t identifier;
	uint32_t apiVersion;
	char driverName[64];

	uint32_t graphicsApiId;

	bool (*Init)(void);
	void (*Term)(void);

	bool (*EnumerateDevices)(uint32_t *count, struct NeRenderDeviceInfo *devices);
	struct NeRenderDevice *(*CreateDevice)(struct NeRenderDeviceInfo *info,
										 struct NeRenderDeviceProcs *devProcs,
										 struct NeRenderContextProcs *ctxProcs);
	void (*DestroyDevice)(struct NeRenderDevice *dev);
};

typedef const struct NeRenderDriver *(*ReLoadDriverProc)(void);

ENGINE_API extern const struct NeRenderDriver *Re_driver;

#ifdef RE_NATIVE_VULKAN
#include <vulkan/vulkan.h>

struct NeRenderInterface
{
	VkCommandBuffer (*CurrentCommandBuffer)(struct NeRenderContext *);
	
	VkSemaphore frameSemaphore;
	uint64_t (*FrameSemaphoreValue)(struct NeRenderDevice *);

	VkSemaphore (*SemaphoreHandle)(struct NeSemaphore *);
	uint64_t (*CurrentSemaphoreValue)(struct NeSemaphore *);
	
	VkImage (*Image)(struct NeTexture *);
	VkImageView (*ImageView)(struct NeTexture *);
	VkBuffer (*Buffer)(struct NeBuffer *);
	VkAccelerationStructureKHR (*AccelerationStructure)(struct NeAccelerationStructure *);
	VkFramebuffer (*Framebuffer)(struct NeFramebuffer *);
	VkRenderPass (*RenderPass)(struct NeRenderPassDesc *);
	VkSampler (*Sampler)(struct NeSampler *);
	VkPipeline (*Pipeline)(struct NePipeline *);

	VkDevice device;
	VkQueue graphicsQueue, computeQueue, transferQueue;
	uint32_t graphicsFamily, computeFamily, transferFamily;
	VkPhysicalDevice physicalDevice;
	VkPipelineCache pipelineCache;
	VkAllocationCallbacks *allocationCallbacks;

	VkInstance instance;
	PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
};
#endif

#ifdef RE_NATIVE_METAL
#import <Metal/Metal.h>

struct NeRenderInterface
{
	id<MTLCommandQueue> (*CommandQueue)(struct NeRenderContext *);
	id<MTLCommandBuffer> (*CurrentCommandBuffer)(struct NeRenderContext *);
	id<MTLBlitCommandEncoder> (*CurrentBlitEncoder)(struct NeRenderContext *);
	id<MTLRenderCommandEncoder> (*CurrentRenderEncoder)(struct NeRenderContext *);
	id<MTLComputeCommandEncoder> (*CurrentComputeEncoder)(struct NeRenderContext *);

	id<MTLTexture> (*Texture)(struct NeTexture *);
	id<MTLBuffer> (*Buffer)(struct NeBuffer *);
	id<MTLAccelerationStructure> (*AccelerationStructure)(struct NeAccelerationStructure *);
	const MTLRenderPassDescriptor *(*RenderPassDescriptor)(struct NeRenderPassDesc *);

	id<MTLDevice> device;
};
#endif

#ifdef RE_NATIVE_DIRECT3D12
#include <d3d12.h>

struct NeRenderInterface
{
	ID3D12Device5 *device;
	ID3D12CommandQueue *graphicsQueue, *copyQueue, *computeQueue;
	IDXGIAdapter1 *adapter;
};
#endif

#ifdef RE_NATIVE_OPENGL
#include <GL/gl.h>

struct NeRenderInterface
{
	void *context;
};
#endif

#endif /* _NE_RENDER_DRIVER_DRIVER_H_ */
