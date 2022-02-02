#ifndef _NE_RENDER_DRIVER_DEVICE_H_
#define _NE_RENDER_DRIVER_DEVICE_H_

#include <Render/Types.h>

#define	RE_VENDOR_ID_NVIDIA			0x10DE
#define RE_VENDOR_ID_ATI_AMD		0x1002
#define RE_VENDOR_ID_INTEL			0x8086
#define RE_VENDOR_ID_S3				0x5333
#define RE_VENDOR_ID_MATROX			0x102B
#define RE_VENDOR_ID_3DLABS			0x3D3D
#define RE_VENDOR_ID_APPLE			0x106B
#define RE_VENDOR_ID_ARM			0x13B5
#define RE_VENDOR_ID_QUALCOMM		0x5143
#define RE_VENDOR_ID_IMAGINATION	0x1010
#define RE_VENDOR_ID_NEC			0x1033
#define RE_VENDOR_ID_STMICRO		0x104A
#define RE_VENDOR_ID_BROADCOM		0x14E4

struct NeRenderDeviceInfo
{
	char deviceName[256];
	uint64_t localMemorySize;

	struct {
		bool unifiedMemory;
		bool rayTracing;
		bool indirectRayTracing;
		bool meshShading;
		bool discrete;
		bool canPresent;
		bool drawIndirectCount;
		bool bcTextureCompression;
		bool astcTextureCompression;
		bool multiDrawIndirect;
		bool secondaryCommandBuffers;
		bool coherentMemory;
	} features;

	struct {
		uint32_t maxTextureSize;
	} limits;

	struct {
		uint32_t vendorId;
		uint32_t deviceId;
		uint32_t driverVersion;
	} hardwareInfo;

	void *private;
};

struct NeRenderDeviceProcs
{
	uint64_t (*OffsetAddress)(uint64_t address, uint64_t offset);
	uint64_t (*AccelerationStructureHandle)(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as);
	uint64_t (*BufferAddress)(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset);

	bool (*WaitSemaphore)(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value, uint64_t timeout);
	bool (*WaitSemaphores)(struct NeRenderDevice *dev, uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout);
	bool (*SignalSemaphore)(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value);

	void (*ResetContext)(struct NeRenderDevice *dev, struct NeRenderContext *ctx);

	struct NeTexture *(*CreateTransientTexture)(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
	struct NeBuffer *(*CreateTransientBuffer)(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);

	struct NePipeline *(*GraphicsPipeline)(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc);
	struct NePipeline *(*ComputePipeline)(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc);
	struct NePipeline *(*RayTracingPipeline)(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth);

	void (*SetAttachment)(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex);
	
	void *(*AcquireNextImage)(struct NeRenderDevice *dev, struct NeSwapchain *swapchain);
	struct NeTexture *(*SwapchainTexture)(struct NeSwapchain *swapchain, void *image);
	enum NeTextureFormat (*SwapchainFormat)(struct NeSwapchain *swapchain);
	void (*SwapchainDesc)(struct NeSwapchain *swapchain, struct NeFramebufferAttachmentDesc *desc);
	bool (*Present)(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *swapchain, void *image, struct NeSemaphore *wait);

	bool (*Execute)(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait);
	
	struct NeSemaphore *(*CreateSemaphore)(struct NeRenderDevice *dev);
	void (*DestroySemaphore)(struct NeRenderDevice *dev, struct NeSemaphore *semaphore);
	
	struct NeFence *(*CreateFence)(struct NeRenderDevice *dev, bool createSignaled);
	void (*SignalFence)(struct NeRenderDevice *dev, struct NeFence *fence);
	bool (*WaitForFence)(struct NeRenderDevice *dev, struct NeFence *fence, uint64_t timeout);
	void (*DestroyFence)(struct NeRenderDevice *dev, struct NeFence *fence);

	void *(*MapBuffer)(struct NeRenderDevice *dev, struct NeBuffer *buff);
	void (*FlushBuffer)(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size);
	void (*UnmapBuffer)(struct NeRenderDevice *dev, struct NeBuffer *buff);

	struct NeTexture *(*CreateTexture)(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location);
	enum NeTextureLayout (*TextureLayout)(const struct NeTexture *tex);
	void (*DestroyTexture)(struct NeRenderDevice *dev, struct NeTexture *tex);

	struct NeSampler *(*CreateSampler)(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc);
	void (*DestroySampler)(struct NeRenderDevice *dev, struct NeSampler *s);

	struct NeBuffer *(*CreateBuffer)(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location);
	void (*UpdateBuffer)(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size);
	const struct NeBufferDesc *(*NeBufferDesc)(const struct NeBuffer *buff);
	void (*DestroyBuffer)(struct NeRenderDevice *dev, struct NeBuffer *buff);

	struct NeAccelerationStructure *(*CreateAccelerationStructure)(struct NeRenderDevice *dev, const struct NeAccelerationStructureCreateInfo *aci);
	void (*DestroyAccelerationStructure)(struct NeRenderDevice *dev, struct NeAccelerationStructure *as);

	struct NeFramebuffer *(*CreateFramebuffer)(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc);
	void (*DestroyFramebuffer)(struct NeRenderDevice *dev, struct NeFramebuffer *fb);

	struct NeRenderPassDesc *(*CreateRenderPassDesc)(struct NeRenderDevice *dev, const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount);
	void (*DestroyRenderPassDesc)(struct NeRenderDevice *dev, struct NeRenderPassDesc *rp);

	void (*LoadPipelineCache)(struct NeRenderDevice *dev);
	void (*SavePipelineCache)(struct NeRenderDevice *dev);
	void (*DestroyPipeline)(struct NeRenderDevice *dev, struct NePipeline *pipeline);

	struct NeRenderContext *(*CreateContext)(struct NeRenderDevice *dev);
	void (*DestroyContext)(struct NeRenderDevice *dev, struct NeRenderContext *ctx);

	bool (*InitTransientHeap)(struct NeRenderDevice *dev, uint64_t size);
	bool (*ResizeTransientHeap)(struct NeRenderDevice *dev, uint64_t size);
	void (*TermTransientHeap)(struct NeRenderDevice *dev);

	struct NeSurface *(*CreateSurface)(struct NeRenderDevice *dev, void *window);
	void (*DestroySurface)(struct NeRenderDevice *dev, struct NeSurface *surface);

	struct NeSwapchain *(*CreateSwapchain)(struct NeRenderDevice *dev, struct NeSurface *surface, bool verticalSync);
	void (*DestroySwapchain)(struct NeRenderDevice *dev, struct NeSwapchain *swapchain);

	void *(*ShaderModule)(struct NeRenderDevice *dev, const char *name);

	void (*WaitIdle)(struct NeRenderDevice *dev);
	void (*ScreenResized)(struct NeRenderDevice *dev, struct NeSwapchain *sw);

	struct NeRenderInterface *(*CreateRenderInterface)(struct NeRenderDevice *dev);
	void (*DestroyRenderInterface)(struct NeRenderInterface *iface);
};

ENGINE_API extern struct NeRenderDevice *Re_device;
ENGINE_API extern struct NeRenderDeviceInfo Re_deviceInfo;
ENGINE_API extern struct NeRenderDeviceProcs Re_deviceProcs;

static inline struct NeRenderContext *Re_CreateContext(void) { return Re_deviceProcs.CreateContext(Re_device); }
static inline void Re_ResetContext(struct NeRenderContext *ctx) { Re_deviceProcs.ResetContext(Re_device, ctx); }
static inline void Re_DestroyContext(struct NeRenderContext *ctx) { Re_deviceProcs.DestroyContext(Re_device, ctx); }

static inline struct NeSurface *Re_CreateSurface(void *window) { return Re_deviceProcs.CreateSurface(Re_device, window); }
static inline void Re_DestroySurface(struct NeSurface *surface) { Re_deviceProcs.DestroySurface(Re_device, surface); }

static inline void Re_WaitIdle(void) { Re_deviceProcs.WaitIdle(Re_device); }

static inline uint64_t Re_OffsetAddress(uint64_t addr, uint64_t offset) { return Re_deviceProcs.OffsetAddress(addr, offset); }

#endif /* _NE_RENDER_DRIVER_DEVICE_H_ */
