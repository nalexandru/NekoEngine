#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Render.h>
#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Engine/Version.h>
#include <Engine/Application.h>

#include "VKRender.h"

#define VKRMOD	L"VulkanRender"

#if defined(_WIN32)
#	define EXPORT	__declspec(dllexport)
#else
#	define EXPORT
#endif

struct RenderDevice Re_Device = { 0 };
struct RenderWorker Re_MainThreadWorker = { 0 };
struct RenderWorker *Re_Workers = NULL;

VkInstance VK_Instance = VK_NULL_HANDLE;
VkAllocationCallbacks *VK_CPUAllocator = NULL;

static const char *_instExt[10] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
	NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL
};
static uint32_t _instExtCount = 2;

static const char *_instLayers[10] =
{
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL
};
static uint32_t _instLayerCount = 0;

static uint32_t _workerKey;
static Array _transferSubmit, _computeSubmit, _graphicsSubmit, _submitWaitStages;

const char *_validationLayer = "VK_LAYER_KHRONOS_validation";

// Defined in the platform file
extern const char *__VK_PlatformSurfaceExtension;

static bool _Init(void);
static void _Term(void);
static void _WaitIdle(void);
static void _ScreenResized(void);
static void _RenderFrame(void);
void _InitWorker(int worker, struct RenderWorker *w);
void _ResetWorker(struct RenderWorker *w);
void _TermWorker(struct RenderWorker *w);

EXPORT uint32_t Re_ApiVersion = RE_API_VERSION;
EXPORT bool
Re_InitLibrary(void)
{
	memset(&Re, 0x0, sizeof(Re));

	Re.Init = _Init;
	Re.Term = _Term;
	Re.WaitIdle = _WaitIdle;
	Re.ScreenResized = _ScreenResized;
	Re.RenderFrame = _RenderFrame;
	Re.InitScene = VK_InitScene;
	Re.TermScene = VK_TermScene;
	Re.GetShader = VK_GetShader;
	Re.InitTexture = VK_InitTexture;
	Re.UpdateTexture = VK_UpdateTexture;
	Re.TermTexture = VK_TermTexture;
	Re.InitModel = VK_InitModel;
	Re.TermModel = VK_TermModel;

	Re.sceneRenderDataSize = sizeof(struct SceneRenderData);
	Re.modelRenderDataSize = sizeof(struct ModelRenderData);
	Re.textureRenderDataSize = sizeof(struct TextureRenderData);

	return true;
}

bool
_Init(void)
{
	uint32_t apiVersion;

	VkResult rc = volkInitialize();
	if (rc != VK_SUCCESS) {
		Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Failed to load the Vulkan loader. Is the driver installed ?");
		return false;
	}

	rc = vkEnumerateInstanceVersion(&apiVersion);
	if (rc != VK_SUCCESS) {
		Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Failed to retrieve Vulkan instance version. Is the driver installed ?");
		return false;
	}

	if (apiVersion < VK_VERSION_1_2) {
		Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Vulkan 1.2 is required, but the system has %d.%d", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion));
		return false;
	}

	Sys_LogEntry(VKRMOD, LOG_INFORMATION, L"Vulkan Instance %d.%d.%d", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));

	char *appName = Sys_Alloc(sizeof(char), wcslen(App_ApplicationInfo.name), MH_Transient);
	wcstombs(appName, App_ApplicationInfo.name, wcslen(App_ApplicationInfo.name));
	VkApplicationInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = appName,
		.applicationVersion = VK_MAKE_VERSION(App_ApplicationInfo.version.major, App_ApplicationInfo.version.minor, App_ApplicationInfo.version.build),
		.pEngineName = "NekoEngine",
		.engineVersion = VK_MAKE_VERSION(E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD),
		.apiVersion = VK_VERSION_1_2
	};
	VkInstanceCreateInfo ici =
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &ai,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = _instLayers,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = _instExt
	};

	_instExt[_instExtCount++] = __VK_PlatformSurfaceExtension;

	ici.enabledLayerCount = _instLayerCount;
	ici.enabledExtensionCount = _instExtCount;

	rc = vkCreateInstance(&ici, VK_CPUAllocator, &VK_Instance);
	if (rc != VK_SUCCESS) {
		Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Failed to create Vulkan 1.2 instance");
		return false;
	}

	volkLoadInstance(VK_Instance);

	if (!VK_InitDevice())
		return false;

	// If these extensions are not present, InitDevice will fail
	Re.features.rayTracing = true;
	Re.features.meshShading = true;

	if (!VK_InitTransientHeap())
		return false;

	Rt_InitArray(&_transferSubmit, 10, sizeof(VkSubmitInfo));
	Rt_InitArray(&_computeSubmit, 10, sizeof(VkSubmitInfo));
	Rt_InitArray(&_graphicsSubmit, 10, sizeof(VkSubmitInfo));
	Rt_InitArray(&_submitWaitStages, 30, sizeof(VkPipelineStageFlags));

	_workerKey = Sys_TlsAlloc();

	Re_Workers = (struct RenderWorker *)calloc(E_JobWorkerThreads(), sizeof(*Re_Workers));
	if (!Re_Workers)
		return false;

	_InitWorker(E_JobWorkerThreads(), &Re_MainThreadWorker);

	void **dispatchArgs = (void **)Sys_Alloc(sizeof(*dispatchArgs), E_JobWorkerThreads(), MH_Transient);
	for (int i = 0; i < E_JobWorkerThreads(); ++i)
		dispatchArgs[i] = &Re_Workers[i];

	E_DispatchJobs(E_JobWorkerThreads(), (JobProc)_InitWorker, dispatchArgs, NULL);

	if (!VK_CreateSurface())
		return false;

	if (!VK_CreateSwapchain())
		return false;

/*	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(GET_DRAWABLES_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)D3D9_GetDrawables, 0);*/

	return true;
}

void
_WaitIdle(void)
{
	vkDeviceWaitIdle(Re_Device.dev);
}

void
_ScreenResized(void)
{

}

void
_RenderFrame(void)
{
	uint32_t imageId;
	VkResult rc;

	{ // Begin frame
		rc = vkAcquireNextImageKHR(Re_Device.dev, VK_Swapchain.sw, UINT64_MAX, Re_Device.frameStart, VK_NULL_HANDLE, &imageId);
		switch (rc) {
		case VK_SUCCESS: break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR: exit(0); break;
		default: Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Failed to acquire image: %d", rc); E_Shutdown(); return;
		}
	}

	{ // Submit buffers
		if (_transferSubmit.count) {
			rc = vkQueueSubmit(Re_Device.transferQueue, (uint32_t)_transferSubmit.count, (VkSubmitInfo *)_transferSubmit.data, VK_NULL_HANDLE);
			if (rc != VK_SUCCESS) {
				Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Transfer submit failed: %d", rc); E_Shutdown(); return;
				E_Shutdown();
				return;
			}
		}

		if (_computeSubmit.count) {
			rc = vkQueueSubmit(Re_Device.computeQueue, (uint32_t)_computeSubmit.count, (VkSubmitInfo *)_computeSubmit.data, VK_NULL_HANDLE);
			if (rc != VK_SUCCESS) {
				Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Compute submit failed: %d", rc); E_Shutdown(); return;
				E_Shutdown();
				return;
			}
		}

		if (_graphicsSubmit.count) {
			rc = vkQueueSubmit(Re_Device.graphicsQueue, (uint32_t)_graphicsSubmit.count, (VkSubmitInfo *)_graphicsSubmit.data, VK_NULL_HANDLE);
			if (rc != VK_SUCCESS) {
				Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Graphics submit failed: %d", rc); E_Shutdown(); return;
				E_Shutdown();
				return;
			}
		}
	}

	{ // End frame
		VkPresentInfoKHR pi =
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &Re_Device.frameEnd,
			.swapchainCount = 1,
			.pSwapchains = &VK_Swapchain.sw,
			.pImageIndices = &imageId
		};
		rc = vkQueuePresentKHR(Re_Device.graphicsQueue, &pi);
		switch (rc) {
		case VK_SUCCESS: break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR: exit(0); break;
		default: Sys_LogEntry(VKRMOD, LOG_CRITICAL, L"Failed to present image: %d", rc); E_Shutdown(); return;
		}

		Re_Device.frame = (Re_Device.frame + 1) % RE_NUM_BUFFERS;

		Rt_ClearArray(&_transferSubmit, false);
		Rt_ClearArray(&_computeSubmit, false);
		Rt_ClearArray(&_graphicsSubmit, false);
		Rt_ClearArray(&_submitWaitStages, false);
	}
}

void
_Term(void)
{
	if (VK_Instance == VK_NULL_HANDLE)
		return;

	vkDeviceWaitIdle(Re_Device.dev);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
		_TermWorker(&Re_Workers[i]);
	_TermWorker(&Re_MainThreadWorker);

	VK_TermTransientHeap();

	VK_TermDevice();

	vkDestroyInstance(VK_Instance, VK_CPUAllocator);
}

struct RenderWorker *
VK_CurrentThreadWorker(void)
{
	return Sys_TlsGet(_workerKey);
}

VkCommandBuffer
VK_GraphicsCommandBuffer(int worker, VkCommandBufferLevel level)
{
	struct RenderWorker *w = Sys_TlsGet(_workerKey);

	return VK_NULL_HANDLE;
}

VkCommandBuffer
VK_TransferCommandBuffer(int worker, VkCommandBufferLevel level)
{
	struct RenderWorker *w = Sys_TlsGet(_workerKey);

	return VK_NULL_HANDLE;
}

VkCommandBuffer
VK_ComputeCommandBuffer(int worker, VkCommandBufferLevel level)
{
	struct RenderWorker *w = Sys_TlsGet(_workerKey);

	return VK_NULL_HANDLE;
}

void
_InitWorker(int worker, struct RenderWorker *w)
{
	VkCommandPoolCreateInfo ci =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	};

	Sys_TlsSet(_workerKey, w);

	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		ci.queueFamilyIndex = Re_Device.graphicsQueueFamily;
		vkCreateCommandPool(Re_Device.dev, &ci, NULL, &w->graphicsCmdPools[i]);

		ci.queueFamilyIndex = Re_Device.computeQueueFamily;
		vkCreateCommandPool(Re_Device.dev, &ci, NULL, &w->computeCmdPools[i]);
		
		ci.queueFamilyIndex = Re_Device.transferQueueFamily;
		vkCreateCommandPool(Re_Device.dev, &ci, NULL, &w->transferCmdPools[i]);
	}
}

void
_ResetWorker(struct RenderWorker *w)
{
	//
}

void
_TermWorker(struct RenderWorker *w)
{
	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		vkDestroyCommandPool(Re_Device.dev, w->graphicsCmdPools[i], NULL);
		vkDestroyCommandPool(Re_Device.dev, w->computeCmdPools[i], NULL);
		vkDestroyCommandPool(Re_Device.dev, w->transferCmdPools[i], NULL);
	}
}

