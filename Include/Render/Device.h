#ifndef _RE_DEVICE_H_
#define _RE_DEVICE_H_

#include <Engine/Types.h>
#include <Render/Context.h>

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
		bool textureCompression;
	} features;
	
	struct {
		uint32_t maxTextureSize;
	} limits;
		
	void *private;
};

struct RenderDeviceProcs
{
	struct Pipeline *(*GraphicsPipeline)(struct RenderDevice *dev, const struct GraphicsPipelineDesc *desc);
	struct Pipeline *(*ComputePipeline)(struct RenderDevice *dev, struct Shader *sh);
	struct Pipeline *(*RayTracingPipeline)(struct RenderDevice *dev, struct ShaderBindingTable *sbt, uint32_t maxDepth);
	
	void (*SetAttachment)(struct Framebuffer *fb, uint32_t pos, struct Texture *tex);
	
	struct PipelineLayout *(*CreatePipelineLayout)(struct RenderDevice *dev, const struct PipelineLayoutDesc *desc);
	void (*DestroyPipelineLayout)(struct RenderDevice *dev, struct PipelineLayout *layout);
	
	void *(*AcquireNextImage)(struct RenderDevice *dev, Swapchain swapchain);
	struct Texture *(*SwapchainTexture)(Swapchain swapchain, void *image);
	enum TextureFormat (*SwapchainFormat)(Swapchain swapchain);
	bool (*Present)(struct RenderDevice *dev, struct RenderContext *ctx, Swapchain swapchain, void *image);

	bool (*Execute)(struct RenderDevice *dev, struct RenderContext *ctx, bool wait);
	
	struct Texture *(*CreateTexture)(struct RenderDevice *dev, const struct TextureCreateInfo *tci);
	const struct TextureDesc *(*TextureDesc)(const struct Texture *tex);
	enum TextureLayout (*TextureLayout)(const struct Texture *tex);
	void (*DestroyTexture)(struct RenderDevice *dev, struct Texture *tex);
	
	struct Sampler *(*CreateSampler)(struct RenderDevice *dev, const struct SamplerDesc *desc);
	void (*DestroySampler)(struct RenderDevice *dev, struct Sampler *s);
	
	struct Buffer *(*CreateBuffer)(struct RenderDevice *dev, const struct BufferCreateInfo *bci);
	void (*UpdateBuffer)(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size);
	const struct BufferDesc *(*BufferDesc)(const struct Buffer *buff);
	void (*DestroyBuffer)(struct RenderDevice *dev, struct Buffer *buff);
	
	struct AccelerationStructure *(*CreateAccelerationStructure)(struct RenderDevice *dev, const struct AccelerationStructureCreateInfo *aci);
	void (*DestroyAccelerationStructure)(struct RenderDevice *dev, struct AccelerationStructure *as);
	
	struct DescriptorSetLayout *(*CreateDescriptorSetLayout)(struct RenderDevice *dev, const struct DescriptorSetLayoutDesc *desc);
	void (*DestroyDescriptorSetLayout)(struct RenderDevice *dev, struct DescriptorSetLayout *dsl);
	
	struct DescriptorSet *(*CreateDescriptorSet)(struct RenderDevice *dev, const struct DescriptorSetLayout *layout);
	void (*CopyDescriptorSet)(struct RenderDevice *dev, const struct DescriptorSet *src, uint32_t srcOffset, struct DescriptorSet *dst, uint32_t dstOffset, uint32_t count);
	void (*WriteDescriptorSet)(struct RenderDevice *dev, struct DescriptorSet *ds, const struct DescriptorWrite *writes, uint32_t writeCount);
	void (*DestroyDescriptorSet)(struct RenderDevice *dev, struct DescriptorSet *ds);
	
	struct Framebuffer *(*CreateFramebuffer)(struct RenderDevice *dev, const struct FramebufferDesc *desc);
	void (*DestroyFramebuffer)(struct RenderDevice *dev, struct Framebuffer *fb);
	
	struct RenderPass *(*CreateRenderPass)(struct RenderDevice *dev, const struct RenderPassDesc *desc);
	void (*DestroyRenderPass)(struct RenderDevice *dev, struct RenderPass *rp);

	void (*LoadPipelineCache)(struct RenderDevice *dev);
	void (*SavePipelineCache)(struct RenderDevice *dev);
	
	struct RenderContext *(*CreateContext)(struct RenderDevice *dev);
	void (*DestroyContext)(struct RenderDevice *dev, struct RenderContext *ctx);
	
	Surface (*CreateSurface)(struct RenderDevice *dev, void *window);
	void (*DestroySurface)(struct RenderDevice *dev, Surface surface);
	
	Swapchain (*CreateSwapchain)(struct RenderDevice *dev, Surface surface);
	void (*DestroySwapchain)(struct RenderDevice *dev, Swapchain swapchain);
	
	void *(*ShaderModule)(struct RenderDevice *dev, const char *name);

	void (*WaitIdle)(struct RenderDevice *dev);
};

extern struct RenderDevice *Re_device;
extern struct RenderDeviceInfo Re_deviceInfo;
extern struct RenderDeviceProcs Re_deviceProcs;

static inline struct RenderContext *Re_CreateContext(void) { return Re_deviceProcs.CreateContext(Re_device); }
static inline void Re_DestroyContext(struct RenderContext *ctx) { Re_deviceProcs.DestroyContext(Re_device, ctx); }

static inline Surface Re_CreateSurface(void *window) { return Re_deviceProcs.CreateSurface(Re_device, window); }
static inline void Re_DestroySurface(Surface surface) { Re_deviceProcs.DestroySurface(Re_device, surface); }

static inline void Re_WaitIdle(void) { Re_deviceProcs.WaitIdle(Re_device); }

#endif /* _RE_DEVICE_H_ */
