#include <stdlib.h>
#include <string.h>

#include <System/Log.h>
#include <System/System.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Driver.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <Render/Context.h>
#include <Render/Pipeline.h>
#include <Render/Swapchain.h>

#define RE_MOD L"Render"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(RE_MOD, LOG_CRITICAL, y); return false; }

struct RenderDevice *Re_device;
struct RenderDeviceInfo Re_deviceInfo = { 0 };
struct RenderDeviceProcs Re_deviceProcs = { 0 };
struct RenderContextProcs Re_contextProcs = { 0 };

void *Re_surface = NULL;
void *Re_swapchain = NULL;
struct RenderContext **Re_contexts = NULL;
const struct RenderDriver *Re_driver = NULL;

#ifndef RENDER_DRIVER_BUILTIN
static void *_drvModule = NULL;
#endif

bool
Re_InitRender(void)
{
	uint32_t devCount = 0;
	struct RenderDeviceInfo *info = NULL, *selected = NULL;

	if (E_GetCVarBln(L"Render_WaitForDebugger", false)->bln)
		Sys_MessageBox(L"NekoEngine", L"Attach the graphics debugger now", MSG_ICON_INFO);

#ifdef RENDER_DRIVER_BUILTIN
	Re_driver = Re_LoadBuiltinDriver();
#else
	ReLoadDriverProc loadDriver;
	const char *drvPath = E_GetCVarStr(L"Render_Driver", "VulkanDriver")->str;
	_drvModule = Sys_LoadLibrary(drvPath);
	CHK_FAIL(_drvModule, L"Failed to load driver module");

	loadDriver = Sys_GetProcAddress(_drvModule, "Re_LoadDriver");
	CHK_FAIL(loadDriver, L"The library is not a valid driver");

	Re_driver = loadDriver();
#endif
	
	CHK_FAIL(Re_driver, L"Failed to load driver");
	CHK_FAIL((Re_driver->identifier == NE_RENDER_DRIVER_ID), L"The library is not a valid driver");
	CHK_FAIL((Re_driver->apiVersion == NE_RENDER_DRIVER_API), L"Driver version mismatch");
	CHK_FAIL(Re_driver->Init(), L"Failed to initialize driver");
	CHK_FAIL(Re_driver->EnumerateDevices(&devCount, NULL), L"Failed to enumerate devices");
	
	info = Sys_Alloc(sizeof(*info), devCount, MH_Transient);
	CHK_FAIL(info, L"Failed to enumerate devices");
	CHK_FAIL(Re_driver->EnumerateDevices(&devCount, info), L"Failed to enumerate devices");
	
	uint64_t vramSize = 0;
	bool haveRt = false;
	for (uint32_t i = 0; i < devCount; ++i) {
		if (!info[i].features.canPresent)
			continue;
		
		if (info[i].features.rayTracing) {
			if (!haveRt || (haveRt && vramSize < info[i].localMemorySize))
				goto updateSelection;
		} else {
			if (vramSize < info[i].localMemorySize)
				goto updateSelection;
		}
		
		continue;
		
	updateSelection:
		selected = &info[i];
		vramSize = info[i].localMemorySize;
		haveRt = info[i].features.rayTracing;
	}
	
	CHK_FAIL(selected, L"No suitable device found");
	
	memcpy(&Re_deviceInfo, selected, sizeof(Re_deviceInfo));
	
	Re_device = Re_driver->CreateDevice(&Re_deviceInfo, &Re_deviceProcs, &Re_contextProcs);
	CHK_FAIL(Re_device, L"Failed to create device");
	
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"GPU: %hs (%ls)", Re_deviceInfo.deviceName, Re_driver->driverName);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tMemory: %llu MB", Re_deviceInfo.localMemorySize / 1024 / 1024);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tRay Tracing: %ls", Re_deviceInfo.features.rayTracing ? L"yes" : L"no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tMesh Shading: %ls", Re_deviceInfo.features.rayTracing ? L"yes" : L"no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tUnified Memory: %ls", Re_deviceInfo.features.unifiedMemory ? L"yes" : L"no");
	
	Re_surface = Re_CreateSurface(E_screen);
	CHK_FAIL(Re_surface, L"Failed to create surface");
	
	Re_swapchain = Re_CreateSwapchain(Re_surface);
	CHK_FAIL(Re_swapchain, L"Failed to create swapchain");
	
	CHK_FAIL(Re_LoadShaders(), L"Failed to load shaders");
	CHK_FAIL(Re_InitPipelines(), L"Failed to create pipelines");
	
	Re_contexts = calloc(1, sizeof(*Re_contexts));
	CHK_FAIL(Re_contexts, L"Failed to allocate contexts");
	
	Re_contexts[0] = Re_CreateContext();
	CHK_FAIL(Re_contexts[0], L"Failed to create context");
	
	//for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i) {

	//}

	CHK_FAIL(Re_InitTextureSystem(), L"Failed to initialize texture system");
	
	CHK_FAIL(E_RegisterResourceType(RES_TEXTURE, sizeof(struct TextureResource), (ResourceCreateProc)Re_CreateTextureResource,
							(ResourceLoadProc)Re_LoadTextureResource, (ResourceUnloadProc)Re_UnloadTextureResource),
			L"Failed to register texture resource");
	CHK_FAIL(E_RegisterResourceType(RES_MODEL, sizeof(struct Model), (ResourceCreateProc)Re_CreateModelResource,
							(ResourceLoadProc)Re_LoadModelResource, (ResourceUnloadProc)Re_UnloadModelResource),
			L"Failed to register model resource");

	return true;
}

void
Re_TermRender(void)
{
	Re_WaitIdle();
	
	Re_TermTextureSystem();

	Re_DestroyContext(Re_contexts[0]);
	free(Re_contexts);
	
	Re_TermPipelines();
	
	Re_UnloadShaders();
	
	Re_DestroySwapchain(Re_swapchain);
	Re_DestroySurface(Re_surface);

	Re_driver->DestroyDevice(Re_device);
	Re_driver->Term();

#ifndef RENDER_DRIVER_BUILTIN
	Sys_UnloadLibrary(_drvModule);
#endif
}
