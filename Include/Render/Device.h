#ifndef _RE_DEVICE_H_
#define _RE_DEVICE_H_

#include <Engine/Types.h>

struct RenderDeviceInfo
{
	char deviceName[256];
	uint64_t localMemorySize;
	
	struct {
		bool unifiedMemory;
		bool rayTracing;
		bool meshShading;
		bool discrete;
		bool canPresent;
	} features;
	
	struct {
		uint32_t maxTextureSize;
	} limits;
		
	void *private;
};

struct RenderDeviceProcs
{
	struct Pipeline *(*GraphicsPipeline)(struct RenderDevice *dev, struct Shader *sh, uint64_t flags, struct BlendAttachmentDesc *at, uint32_t atCount);
	struct Pipeline *(*ComputePipeline)(struct RenderDevice *dev, struct Shader *sh);
	struct Pipeline *(*RayTracingPipeline)(struct RenderDevice *dev, struct ShaderBindingTable *sbt);
	
	void *(*AcquireNextImage)(struct RenderDevice *dev, void *swapchain);
	bool (*Present)(struct RenderDevice *dev, void *swapchain, void *image);
	
	struct Texture *(*CreateTexture)(struct RenderDevice *dev, const struct TextureCreateInfo *tci);
	void (*DestroyTexture)(struct RenderDevice *dev, struct Texture *tex);
	
	struct Texture *(*CreateBuffer)(struct RenderDevice *dev, const struct BufferCreateInfo *bci);
	void (*DestroyBuffer)(struct RenderDevice *dev, struct Buffer *buff);
	
	struct AccelerationStructure *(*CreateAccelerationStructure)(struct RenderDevice *dev, const struct AccelerationStructureCreateInfo *aci);
	void (*DestroyAccelerationStructure)(struct RenderDevice *dev, struct AccelerationStructure *as);
	
	void (*LoadPipelineCache)(struct RenderDevice *dev, struct Stream *stm);
	void (*SavePipelineCache)(struct RenderDevice *dev, struct Stream *stm);
	
	bool (*Init)(struct RenderDevice *dev);
	void (*Term)(struct RenderDevice *dev);
	
	struct RenderContext *(*CreateContext)(struct RenderDevice *dev);
	void (*DestroyContext)(struct RenderDevice *dev, struct RenderContext *ctx);
	
	void *(*CreateSurface)(struct RenderDevice *dev, void *window);
	void (*DestroySurface)(struct RenderDevice *dev, void *surface);
	
	void *(*CreateSwapchain)(struct RenderDevice *dev, void *surface);
	void (*DestroySwapchain)(struct RenderDevice *dev, void *swapchain);
};

extern struct RenderDevice *Re_Device;
extern struct RenderDeviceInfo Re_DeviceInfo;
extern struct RenderDeviceProcs Re_DeviceProcs;

static inline bool Re_InitDevice(void) { return Re_DeviceProcs.Init(Re_Device); }

static inline void *Re_AcquireNextImage(struct RenderDevice *dev, void *swapchain) { return Re_DeviceProcs.AcquireNextImage(dev, swapchain); }
static inline bool Present(struct RenderDevice *dev, void *swapchain, void *image) { return Re_DeviceProcs.Present(dev, swapchain, image); }

static inline struct RenderContext *Re_CreateContext(struct RenderDevice *dev) { return Re_DeviceProcs.CreateContext(dev); }
static inline void Re_DestroyContext(struct RenderDevice *dev, struct RenderContext *ctx) { Re_DeviceProcs.DestroyContext(dev, ctx); }

static inline void *Re_CreateSurface(struct RenderDevice *dev, void *window) { return Re_DeviceProcs.CreateSurface(dev, window); }
static inline void Re_DestroySurface(struct RenderDevice *dev, void *surface) { Re_DeviceProcs.DestroySurface(dev, surface); }

static inline void *Re_CreateSwapchain(struct RenderDevice *dev, void *surface) { return Re_DeviceProcs.CreateSwapchain(dev, surface); }
static inline void Re_DestroySwapchain(struct RenderDevice *dev, void *swapchain) { Re_DeviceProcs.DestroySwapchain(dev, swapchain); }

//static inline bool Re_InitDevice(struct RenderDevice *dev) { return Re_DeviceProcs.Init(dev); }
//static inline bool Re_InitDevice(struct RenderDevice *dev) { return Re_DeviceProcs.Init(dev); }
//static inline bool Re_InitDevice(struct RenderDevice *dev) { return Re_DeviceProcs.Init(dev); }

static inline void Re_TermDevice(void) { Re_DeviceProcs.Term(Re_Device); }

#endif /* _RE_DEVICE_H_ */
