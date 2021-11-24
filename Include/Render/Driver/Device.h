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

struct RenderDeviceInfo
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

struct RenderDeviceProcs
{
	uint64_t (*OffsetAddress)(uint64_t address, uint64_t offset);
	uint64_t (*AccelerationStructureHandle)(struct RenderDevice *dev, const struct AccelerationStructure *as);
	uint64_t (*BufferAddress)(struct RenderDevice *dev, const struct Buffer *buff, uint64_t offset);

	bool (*WaitSemaphore)(struct RenderDevice *dev, struct Semaphore *s, uint64_t value, uint64_t timeout);
	bool (*WaitSemaphores)(struct RenderDevice *dev, uint32_t count, struct Semaphore *s, uint64_t *values, uint64_t timeout);
	bool (*SignalSemaphore)(struct RenderDevice *dev, struct Semaphore *s, uint64_t value);

	void (*ResetContext)(struct RenderDevice *dev, struct RenderContext *ctx);

	struct Texture *(*CreateTransientTexture)(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
	struct Buffer *(*CreateTransientBuffer)(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);

	struct Pipeline *(*GraphicsPipeline)(struct RenderDevice *dev, const struct GraphicsPipelineDesc *desc);
	struct Pipeline *(*ComputePipeline)(struct RenderDevice *dev, const struct ComputePipelineDesc *desc);
	struct Pipeline *(*RayTracingPipeline)(struct RenderDevice *dev, struct ShaderBindingTable *sbt, uint32_t maxDepth);

	void (*SetAttachment)(struct Framebuffer *fb, uint32_t pos, struct Texture *tex);
	
	void *(*AcquireNextImage)(struct RenderDevice *dev, struct Swapchain *swapchain);
	struct Texture *(*SwapchainTexture)(struct Swapchain *swapchain, void *image);
	enum TextureFormat (*SwapchainFormat)(struct Swapchain *swapchain);
	void (*SwapchainDesc)(struct Swapchain *swapchain, struct FramebufferAttachmentDesc *desc);
	bool (*Present)(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *swapchain, void *image, struct Semaphore *wait);

	bool (*Execute)(struct RenderDevice *dev, struct RenderContext *ctx, bool wait);
	
	struct Semaphore *(*CreateSemaphore)(struct RenderDevice *dev);
	void (*DestroySemaphore)(struct RenderDevice *dev, struct Semaphore *semaphore);
	
	struct Fence *(*CreateFence)(struct RenderDevice *dev, bool createSignaled);
	void (*SignalFence)(struct RenderDevice *dev, struct Fence *fence);
	bool (*WaitForFence)(struct RenderDevice *dev, struct Fence *fence, uint64_t timeout);
	void (*DestroyFence)(struct RenderDevice *dev, struct Fence *fence);

	void *(*MapBuffer)(struct RenderDevice *dev, struct Buffer *buff);
	void (*FlushBuffer)(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, uint64_t size);
	void (*UnmapBuffer)(struct RenderDevice *dev, struct Buffer *buff);

	struct Texture *(*CreateTexture)(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location);
	enum TextureLayout (*TextureLayout)(const struct Texture *tex);
	void (*DestroyTexture)(struct RenderDevice *dev, struct Texture *tex);

	struct Sampler *(*CreateSampler)(struct RenderDevice *dev, const struct SamplerDesc *desc);
	void (*DestroySampler)(struct RenderDevice *dev, struct Sampler *s);

	struct Buffer *(*CreateBuffer)(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location);
	void (*UpdateBuffer)(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size);
	const struct BufferDesc *(*BufferDesc)(const struct Buffer *buff);
	void (*DestroyBuffer)(struct RenderDevice *dev, struct Buffer *buff);

	struct AccelerationStructure *(*CreateAccelerationStructure)(struct RenderDevice *dev, const struct AccelerationStructureCreateInfo *aci);
	void (*DestroyAccelerationStructure)(struct RenderDevice *dev, struct AccelerationStructure *as);

	struct Framebuffer *(*CreateFramebuffer)(struct RenderDevice *dev, const struct FramebufferDesc *desc);
	void (*DestroyFramebuffer)(struct RenderDevice *dev, struct Framebuffer *fb);

	struct RenderPassDesc *(*CreateRenderPassDesc)(struct RenderDevice *dev, const struct AttachmentDesc *attachments, uint32_t count, const struct AttachmentDesc *depthAttachment, const struct AttachmentDesc *inputAttachments, uint32_t inputCount);
	void (*DestroyRenderPassDesc)(struct RenderDevice *dev, struct RenderPassDesc *rp);

	void (*LoadPipelineCache)(struct RenderDevice *dev);
	void (*SavePipelineCache)(struct RenderDevice *dev);
	void (*DestroyPipeline)(struct RenderDevice *dev, struct Pipeline *pipeline);

	struct RenderContext *(*CreateContext)(struct RenderDevice *dev);
	void (*DestroyContext)(struct RenderDevice *dev, struct RenderContext *ctx);

	bool (*InitTransientHeap)(struct RenderDevice *dev, uint64_t size);
	bool (*ResizeTransientHeap)(struct RenderDevice *dev, uint64_t size);
	void (*TermTransientHeap)(struct RenderDevice *dev);

	struct Surface *(*CreateSurface)(struct RenderDevice *dev, void *window);
	void (*DestroySurface)(struct RenderDevice *dev, struct Surface *surface);

	struct Swapchain *(*CreateSwapchain)(struct RenderDevice *dev, struct Surface *surface, bool verticalSync);
	void (*DestroySwapchain)(struct RenderDevice *dev, struct Swapchain *swapchain);

	void *(*ShaderModule)(struct RenderDevice *dev, const char *name);

	void (*WaitIdle)(struct RenderDevice *dev);
	void (*ScreenResized)(struct RenderDevice *dev, struct Swapchain *sw);
};

ENGINE_API extern struct RenderDevice *Re_device;
ENGINE_API extern struct RenderDeviceInfo Re_deviceInfo;
ENGINE_API extern struct RenderDeviceProcs Re_deviceProcs;

static inline struct RenderContext *Re_CreateContext(void) { return Re_deviceProcs.CreateContext(Re_device); }
static inline void Re_ResetContext(struct RenderContext *ctx) { Re_deviceProcs.ResetContext(Re_device, ctx); }
static inline void Re_DestroyContext(struct RenderContext *ctx) { Re_deviceProcs.DestroyContext(Re_device, ctx); }

static inline struct Surface *Re_CreateSurface(void *window) { return Re_deviceProcs.CreateSurface(Re_device, window); }
static inline void Re_DestroySurface(struct Surface *surface) { Re_deviceProcs.DestroySurface(Re_device, surface); }

static inline void Re_WaitIdle(void) { Re_deviceProcs.WaitIdle(Re_device); }

static inline uint64_t Re_OffsetAddress(uint64_t addr, uint64_t offset) { return Re_deviceProcs.OffsetAddress(addr, offset); }

#endif /* _NE_RENDER_DRIVER_DEVICE_H_ */
