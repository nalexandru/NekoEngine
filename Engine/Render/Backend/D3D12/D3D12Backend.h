#ifndef D3D12_BACKEND_H
#define D3D12_BACKEND_H

// Build configuration

#include <Engine/BuildConfig.h>

// End build configuration

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>

#define RE_NATIVE_D3D12
#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Render/RayTracing.h>
#include <Runtime/Array.h>
#include <System/Thread.h>
#include <System/Log.h>

#define D3D12BK_MOD	"D3D12Backend"

#define D3D12BK_LOG_ERR(x, hr)																						  \
		Sys_LogEntry(D3D12BK_MOD, LOG_CRITICAL, "%s call from %s, line %d returned 0x%x", x, __FILE__, __LINE__, hr); \
		D3D12Bk_LogDXGIMessages()

extern IDXGIFactory6 *D3D12_factory;

struct D3D12_Fence
{
	ID3D12Fence *fence;
	uint64_t value;
	HANDLE event;
};

struct NeFence
{
	D3D12_Fence df;
};

struct NeSemaphore
{
	D3D12_Fence df;
};

struct NeRenderDevice
{
	ID3D12Device10 *dev;
	ID3D12Heap *transientHeap;
	ID3D12DescriptorHeap *cpuTextureHeap;
	ID3D12DescriptorHeap *gpuDescriptorHeap[RE_NUM_FRAMES];

	ID3D12CommandQueue *direct, *compute, *copy;

//	VkPhysicalDevice physDev;
//	VkPhysicalDeviceProperties physDevProps;
//	VkPhysicalDeviceMemoryProperties physDevMemProps;

	HANDLE frameSemaphore;
	uint64_t semaphoreValue;
	uint64_t *frameValues;
	ID3D12CommandAllocator *copyAllocator;
	ID3D12GraphicsCommandList7 *copyCmdList;
	D3D12_Fence copyFence;

	/*VkDescriptorSetLayout setLayout;
	VkDescriptorPool descriptorPool;

	VkDescriptorSetLayout iaSetLayout;
	VkDescriptorPool iaDescriptorPool[RE_NUM_FRAMES];*/

	ID3D12DescriptorHeap *rtvHeap, *dsvHeap;
	UINT uavIncrement, rtvIncrement, dsvIncrement;

	IDXGIAdapter4 *adapter;
	ID3D12DebugDevice2 *debugDevice;
};

struct NeRenderContext
{
	ID3D12GraphicsCommandList7 *cmdList;

	struct NePipeline *boundPipeline;
	ID3D12CommandAllocator **gfxAllocators, **copyAllocators, **compAllocators;

	struct {
		struct NeArray *gfx, *comp, *copy;
	} closed;

	struct {
		struct NeArray *gfx, *comp, *copy;
	} free;

	ID3D12Fence *executeFence;
	ID3D12Device10 *d3dDev;
	ID3D12DescriptorHeap *descriptorHeap, *iaHeap;
	struct NeRenderDevice *neDev;
	uint32_t lastSubmittedXfer, lastSubmittedCompute;

	struct
	{
		struct NeArray graphics, compute, copy;
	} queued;

	struct NeRenderPassDesc *boundRenderPass;
	struct NeFramebuffer *boundFramebuffer;

	struct NeSemaphore *wait;
};

struct NeSwapchain
{
	IDXGISwapChain3 *chain;
	ID3D12Resource *targets[RE_NUM_FRAMES];
	UINT presentInterval;
	DXGI_SWAP_CHAIN_DESC1 desc;
};

#if ENABLE_OPENXR
struct XrExtProcs
{
	PFN_xrGetVulkanGraphicsRequirements2KHR GetVulkanGraphicsRequirements2;
};

extern struct XrExtProcs Vkd_XRExt;

struct XRSwapchain
{
	struct NeSwapchain vkSwapchain;
	XrSwapchain colorSwapchain, depthSwapchain;
	XrActionSet actionSet;
	XrPath paths[2];
	struct
	{
		XrAction place;
		XrAction pose;
		XrAction exit;
		XrAction vibrateLeft;
		XrAction vibrateRight;
	} actions;
	XrSystemId sysId;
	XrSpace sceneSpace, leftHandSpace, rightHandSpace;
	uint32_t imageCount;
	XrSwapchainImageBaseHeader *colorImages, *depthImages;
};

extern XrInstance Vkd_xrInst;
extern XrSession Vkd_xrSession;
extern XrSessionState Vkd_xrState;
#endif

struct NeBuffer
{
	ID3D12Resource *res;
	D3D12_RESOURCE_STATES state;
	uint64_t size;
	void *staging;
	bool transient;
};

struct NeTexture
{
	ID3D12Resource *res;
	D3D12_RESOURCE_STATES state;
	bool transient;
};

struct NeFramebuffer
{
	ID3D12Resource *attachments[8];
	ID3D12Resource *depthAttachment;
	uint32_t width, height, layers, attachmentCount;
};

struct NeRenderPassDesc
{
	uint32_t attachmentCount, depthAttachmentId;
	DXGI_FORMAT rtvFormats[8];
	float clearValues[8][4];
	enum NeAttachmentLoadOp loadOp[8];
	enum D3D12_RESOURCE_STATES rtvInitialState[8], rtvState[8], rtvFinalState[8];

	DXGI_FORMAT depthFormat;
	enum NeAttachmentLoadOp depthLoadOp;
	float depthClearValue;
	enum D3D12_RESOURCE_STATES depthInitialState, depthState, depthFinalState;

	uint32_t inputAttachments;
};

struct NePipeline
{
	ID3D12PipelineState *ps;
	ID3D12RootSignature *rs;
	NeBuffer *pushConstants;
	D3D_PRIMITIVE_TOPOLOGY topology;
	UINT vertexBufferStride[15];
};

struct NeAccelerationStructure
{
	ID3D12Resource *res, *scratch;
	D3D12_RESOURCE_STATES state;
};

struct D3D12_SubmitInfo
{
	D3D12_Fence *wait, *signal;
	uint64_t waitValue, signalValue;
	ID3D12GraphicsCommandList7 *cmdList;
};

// These are defined in Platform
extern "C" wchar_t *NeWin32_UTF8toUCS2(const char *text);
extern "C" char *NeWin32_UCS2toUTF8(const wchar_t *text);

extern uint32_t D3D12Bk_staticSamplerCount;
extern D3D12_STATIC_SAMPLER_DESC D3D12Bk_staticSamplers[10];

/*extern VkAllocationCallbacks *Vkd_allocCb, *Vkd_transientAllocCb;
extern VkCommandPool Vkd_transferPool;
extern VkPipelineCache Vkd_pipelineCache;
extern struct NeSemaphore Vkd_stagingSignal;
extern uint32_t Vkd_instanceVersion;*/
void D3D12Bk_LogDXGIMessages(void);

// Context
void D3D12Bk_ExecuteCommands(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, struct NeSemaphore *waitSemaphore);

// Descriptor Set
bool D3D12Bk_InitDescriptorHeap(struct NeRenderDevice *dev);
void D3D12Bk_SetSampler(struct NeRenderDevice *dev, uint16_t location, ID3D12Resource *res);
void D3D12Bk_SetTexture(uint16_t location, ID3D12Resource *res);
void D3D12Bk_SetInputAttachment(ID3D12DescriptorHeap *heap, uint16_t location, ID3D12Resource *res);
void D3D12Bk_TermDescriptorSet(struct NeRenderDevice *dev);

// Shader
bool D3D12Bk_LoadShaders();
void D3D12Bk_UnloadShaders();

inline bool
D3D12Bk_InitFence(D3D12_Fence *f, bool signaled)
{
	if (FAILED(Re_device->dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&f->fence))))
		return false;

	f->event = CreateEvent(NULL, FALSE, signaled, NULL);
	if (f->event == INVALID_HANDLE_VALUE)
		return false;

	f->value = 0;

	return true;
}

inline HRESULT
D3D12Bk_SignalFenceCPU(D3D12_Fence *f)
{
	return f->fence->Signal(++f->value);
}

inline bool
D3D12Bk_WaitForFenceCPU(D3D12_Fence *f, DWORD timeout = INFINITE)
{
	DWORD rc = WAIT_FAILED;
	if (f->fence->GetCompletedValue() < f->value) {
		f->fence->SetEventOnCompletion(f->value, f->event);
		rc = WaitForSingleObject(f->event, timeout);
	}

	return rc == WAIT_OBJECT_0;
}

inline bool
D3D12Bk_WaitForFenceCPUExplicit(D3D12_Fence *f, DWORD timeout, uint64_t value)
{
	DWORD rc = WAIT_FAILED;
	if (f->fence->GetCompletedValue() < value) {
		f->fence->SetEventOnCompletion(value, f->event);
		rc = WaitForSingleObject(f->event, timeout);
	}

	return rc == WAIT_OBJECT_0;
}

inline HRESULT
D3D12Bk_SignalFenceGPU(D3D12_Fence *f, ID3D12CommandQueue *queue)
{
	return queue->Signal(f->fence, ++f->value);
}

static inline HRESULT
D3D12Bk_WaitForFenceGPU(D3D12_Fence *f, ID3D12CommandQueue *queue)
{
	return queue->Wait(f->fence, f->value);
}

static inline void
D3D12Bk_TermFence(struct D3D12_Fence *f)
{
	f->fence->Release();
	CloseHandle(f->event);
}

/*static inline VkImageAspectFlags
NeFormatAspect(enum NeTextureFormat fmt)
{
	switch (fmt) {
		case TF_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
		case TF_D24_STENCIL8: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}*/

static inline DXGI_FORMAT
NeTextureToDXGIFormat(enum NeTextureFormat fmt)
{
	switch (fmt) {
		case TF_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TF_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case TF_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
		case TF_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		case TF_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
		case TF_R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case TF_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
		case TF_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case TF_A2R10G10B10_UNORM: return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		case TF_D32_SFLOAT: return DXGI_FORMAT_D32_FLOAT;
		case TF_D24_STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case TF_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
		case TF_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
		case TF_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
		case TF_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
		case TF_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
		case TF_R32_UINT: return DXGI_FORMAT_R32_UINT;
		case TF_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
		case TF_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
		case TF_BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
		case TF_BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
		case TF_BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
		case TF_BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
		/*case TF_ETC2_R8G8B8_UNORM: return DXGI_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		case TF_ETC2_R8G8B8_SRGB: return DXGI_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
		case TF_ETC2_R8G8B8A1_UNORM: return DXGI_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
		case TF_ETC2_R8G8B8A1_SRGB: return DXGI_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
		case TF_EAC_R11_UNORM: return DXGI_FORMAT_EAC_R11_UNORM_BLOCK;
		case TF_EAC_R11_SNORM: return DXGI_FORMAT_EAC_R11_SNORM_BLOCK;
		case TF_EAC_R11G11_UNORM: return DXGI_FORMAT_EAC_R11G11_UNORM_BLOCK;
		case TF_EAC_R11G11_SNORM: return DXGI_FORMAT_EAC_R11G11_SNORM_BLOCK;*/
		default: return DXGI_FORMAT_UNKNOWN;
	}
}

static inline enum NeTextureFormat
DXGIToNeTextureFormat(DXGI_FORMAT fmt)
{
	switch (fmt) {
		case DXGI_FORMAT_R8G8B8A8_UNORM: return TF_R8G8B8A8_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return TF_R8G8B8A8_SRGB;
		case DXGI_FORMAT_B8G8R8A8_UNORM: return TF_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return TF_B8G8R8A8_SRGB;
		case DXGI_FORMAT_R16G16B16A16_UNORM: return TF_R16G16B16A16_UNORM;
		case DXGI_FORMAT_R16G16B16A16_FLOAT: return TF_R16G16B16A16_SFLOAT;
		case DXGI_FORMAT_R32G32B32A32_UINT: return TF_R32G32B32A32_UINT;
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return TF_R32G32B32A32_SFLOAT;
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return TF_A2R10G10B10_UNORM;
		case DXGI_FORMAT_D32_FLOAT: return TF_D32_SFLOAT;
		case DXGI_FORMAT_D24_UNORM_S8_UINT: return TF_D24_STENCIL8;
		case DXGI_FORMAT_R8G8_UNORM: return TF_R8G8_UNORM;
		case DXGI_FORMAT_R16G16_UNORM: return TF_R16G16_UNORM;
		case DXGI_FORMAT_R32G32_UINT: return TF_R32G32_UINT;
		case DXGI_FORMAT_R8_UNORM: return TF_R8_UNORM;
		case DXGI_FORMAT_R16_UNORM: return TF_R16_UNORM;
		case DXGI_FORMAT_R32_UINT: return TF_R32_UINT;
		case DXGI_FORMAT_BC5_UNORM: return TF_BC5_UNORM;
		case DXGI_FORMAT_BC5_SNORM: return TF_BC5_SNORM;
		case DXGI_FORMAT_BC6H_UF16: return TF_BC6H_UF16;
		case DXGI_FORMAT_BC6H_SF16: return TF_BC6H_SF16;
		case DXGI_FORMAT_BC7_UNORM: return TF_BC7_UNORM;
		case DXGI_FORMAT_BC7_UNORM_SRGB: return TF_BC7_SRGB;
		/*case DXGI_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return TF_ETC2_R8G8B8_UNORM;
		case DXGI_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return TF_ETC2_R8G8B8_SRGB;
		case DXGI_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return TF_ETC2_R8G8B8A1_UNORM;
		case DXGI_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return TF_ETC2_R8G8B8A1_SRGB;
		case DXGI_FORMAT_EAC_R11_UNORM_BLOCK: return TF_EAC_R11_UNORM;
		case DXGI_FORMAT_EAC_R11_SNORM_BLOCK: return TF_EAC_R11_SNORM;
		case DXGI_FORMAT_EAC_R11G11_UNORM_BLOCK: return TF_EAC_R11G11_UNORM;
		case DXGI_FORMAT_EAC_R11G11_SNORM_BLOCK: return TF_EAC_R11G11_SNORM;*/
		default: return TF_INVALID;
	}
}

static inline DXGI_FORMAT
NeVertexToDXGIFormat(enum NeVertexFormat fmt)
{
	switch (fmt) {
		case VF_FLOAT: return DXGI_FORMAT_R32_FLOAT;
		case VF_FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
		case VF_FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case VF_FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		default: return DXGI_FORMAT_R32_FLOAT;
	}
}

static inline enum NeVertexFormat
DXGIToNeVertexFormat(DXGI_FORMAT fmt)
{
	switch (fmt) {
		case DXGI_FORMAT_R32_FLOAT: return VF_FLOAT;
		case DXGI_FORMAT_R32G32_FLOAT: return VF_FLOAT2;
		case DXGI_FORMAT_R32G32B32_FLOAT: return VF_FLOAT3;
		case DXGI_FORMAT_R32G32B32A32_FLOAT: return VF_FLOAT4;
		default: return VF_FLOAT;
	}
}

static inline D3D12_RESOURCE_DESC
NeToD3DTextureDesc(const struct NeTextureDesc *desc)
{
	switch(desc->type) {
		//case TT_2D_Multisample: CD3DX12_RESOURCE_DESC1::Tex2D(NeTextureToDXGIFormat(desc->format), desc->width, desc->height, desc->arrayLayers, desc->mipLevels, desc->samples); break;
		case TT_3D: return CD3DX12_RESOURCE_DESC::Tex3D(NeTextureToDXGIFormat(desc->format), desc->width, desc->height, desc->depth, desc->mipLevels); break;
		case TT_Cube: return CD3DX12_RESOURCE_DESC::Tex2D(NeTextureToDXGIFormat(desc->format), desc->width, desc->height, 6, desc->mipLevels); break;
		default: return CD3DX12_RESOURCE_DESC::Tex2D(NeTextureToDXGIFormat(desc->format), desc->width, desc->height, desc->arrayLayers, desc->mipLevels); break;
	}
}

static inline void
NeMemoryTypeToD3DHeapProps(enum NeGPUMemoryType type, D3D12_HEAP_PROPERTIES *desc)
{
	switch (type) {
		case MT_CPU_READ: desc->Type = D3D12_HEAP_TYPE_READBACK; break;
		case MT_CPU_WRITE: desc->Type = D3D12_HEAP_TYPE_UPLOAD; break;
		case MT_CPU_COHERENT: desc->Type = D3D12_HEAP_TYPE_UPLOAD;
		// TODO: benchmark this
		// In Vulkan, it is
		// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		//	desc->Type = D3D12_HEAP_TYPE_CUSTOM;
		//	desc->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		break;
		case MT_GPU_LOCAL:
		default: desc->Type = D3D12_HEAP_TYPE_DEFAULT;
	}
}

/*


static inline VkCommandBuffer
Vkd_AllocateCmdBuffer(VkDevice dev, VkCommandBufferLevel level, VkCommandPool pool)
{
	VkCommandBuffer cmdBuff;
	VkCommandBufferAllocateInfo ai =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = pool,
			.level = level,
			.commandBufferCount = 1
		};
	if (vkAllocateCommandBuffers(dev, &ai, &cmdBuff) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	return cmdBuff;
}

static inline uint32_t
Vkd_MemoryTypeIndex(const struct NeRenderDevice *dev, uint32_t filter, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < dev->physDevMemProps.memoryTypeCount; ++i)
		if (/*(filter & (1 << i)) &&* ((dev->physDevMemProps.memoryTypes[i].propertyFlags & flags) == flags))
			return i;
	return 0;
}

static inline VkCommandBuffer
Vkd_TransferCmdBuffer(struct NeRenderDevice *dev)
{
	VkCommandBuffer cb;

	VkCommandBufferAllocateInfo cbai =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = dev->driverTransferPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
	vkAllocateCommandBuffers(dev->dev, &cbai, &cb);

	VkCommandBufferBeginInfo cbbi =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};
	vkBeginCommandBuffer(cb, &cbbi);

	return cb;
}

static inline VkResult
Vkd_QueueSubmit(struct VkdRenderQueue *q, uint32_t count, const VkSubmitInfo2KHR *submits, VkFence fence)
{
	Sys_LockFutex(q->ftx);
	VkResult rc = vkQueueSubmit2KHR(q->queue, count, submits, fence);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

static inline void
Vkd_ExecuteCmdBuffer(struct NeRenderDevice *dev, VkCommandBuffer cb)
{
	vkEndCommandBuffer(cb);

	VkCommandBufferSubmitInfoKHR cbsi =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
			.commandBuffer = cb
		};
	VkSubmitInfo2KHR si =
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
			.commandBufferInfoCount = 1,
			.pCommandBufferInfos = &cbsi
		};
	Vkd_QueueSubmit(&dev->transfer, 1, &si, dev->driverTransferFence);

	vkWaitForFences(dev->dev, 1, &dev->driverTransferFence, VK_TRUE, UINT64_MAX);
	vkResetFences(dev->dev, 1, &dev->driverTransferFence);

	vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 1, &cb);
}

static inline void
Vkd_TransitionImageLayoutRange(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange *range)
{
	VkPipelineStageFlagBits src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkImageMemoryBarrier barrier =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = *range
		};

	switch (barrier.oldLayout) {
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			src = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			src = VK_PIPELINE_STAGE_TRANSFER_BIT;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			src = VK_PIPELINE_STAGE_TRANSFER_BIT;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			src = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
		default:
			barrier.srcAccessMask = 0;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
	}

	switch (barrier.newLayout) {
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		default:
			barrier.dstAccessMask = 0;
			break;
	}

	vkCmdPipelineBarrier(cmdBuffer, src, dst, 0, 0, NULL, 0, NULL, 1, &barrier);
}

static inline void
Vkd_TransitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageSubresourceRange range =
		{
			.baseMipLevel = 0,
			.baseArrayLayer = 0,
			.levelCount = 1,
			.layerCount = 1
		};
	Vkd_TransitionImageLayoutRange(cmdBuffer, image, oldLayout, newLayout, &range);
}*/

static inline D3D12_RESOURCE_STATES
NeImageLayoutToD3DResourceState(enum NeTextureLayout tl)
{
	switch (tl) {
		case TL_COLOR_ATTACHMENT: return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case TL_DEPTH_STENCIL_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_READ;
		case TL_DEPTH_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case TL_DEPTH_READ_ONLY_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_READ;
		case TL_STENCIL_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case TL_TRANSFER_SRC: return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case TL_TRANSFER_DST: return D3D12_RESOURCE_STATE_COPY_DEST;
		case TL_SHADER_READ_ONLY: return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
		case TL_PRESENT_SRC: return D3D12_RESOURCE_STATE_PRESENT;
		case TL_UNKNOWN:
		default: return D3D12_RESOURCE_STATE_COMMON;
	}
}

static inline enum NeTextureLayout
D3DResourceStateToNeImageLayout(D3D12_RESOURCE_STATES il)
{
	switch (il) {
		case D3D12_RESOURCE_STATE_RENDER_TARGET: return TL_COLOR_ATTACHMENT;
		case D3D12_RESOURCE_STATE_DEPTH_READ: return TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT;
		case D3D12_RESOURCE_STATE_DEPTH_WRITE: return TL_DEPTH_ATTACHMENT;
		case D3D12_RESOURCE_STATE_COPY_SOURCE: return TL_TRANSFER_SRC;
		case D3D12_RESOURCE_STATE_COPY_DEST: return TL_TRANSFER_DST;
		case D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE: return TL_SHADER_READ_ONLY;
		case D3D12_RESOURCE_STATE_PRESENT: return TL_PRESENT_SRC;
		default: return TL_UNKNOWN;
	}
}

#endif /* D3D12_BACKEND_H */

/* NekoEngine
 *
 * D3D12Backend.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
