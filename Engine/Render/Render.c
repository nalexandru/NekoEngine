#include <stdlib.h>
#include <string.h>

#include <System/Log.h>
#include <System/System.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Driver.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Render/Pipeline.h>
#include <Render/Swapchain.h>

#define RE_MOD L"Render"

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
	if (!_drvModule) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to load driver module");
		return false;
	}

	loadDriver = Sys_GetProcAddress(_drvModule, "Re_LoadDriver");
	if (!loadDriver) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"The library is not a valid driver");
		return false;
	}

	Re_driver = loadDriver();
#endif
	
	if (!Re_driver) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to load driver");
		return false;
	}
	
	if (Re_driver->identifier != NE_RENDER_DRIVER_ID) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"The library is not a valid driver");
		return false;
	}
	
	if (Re_driver->apiVersion != NE_RENDER_DRIVER_API) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Driver version mismatch");
		return false;
	}
	
	if (!Re_driver->Init()) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to initialize driver");
		return false;
	}
	
	if (!Re_driver->EnumerateDevices(&devCount, NULL)) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to enumerate devices");
		return false;
	}
	
	info = calloc(sizeof(*info), devCount);
	if (!Re_driver->EnumerateDevices(&devCount, info)) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to enumerate devices");
		return false;
	}
	
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
	
	if (!selected) {
		Sys_MessageBox(L"Fatal Error", L"No suitable graphics device found", MSG_ICON_ERROR);
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"No suitable device found");
		return false;
	}
	
	memcpy(&Re_deviceInfo, selected, sizeof(Re_deviceInfo));
	free(info);
	
	Re_device = Re_driver->CreateDevice(&Re_deviceInfo, &Re_deviceProcs, &Re_contextProcs);
	if (!Re_device) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to create device");
		return false;
	}
	
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"GPU: %hs (%ls)", Re_deviceInfo.deviceName, Re_driver->driverName);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tMemory: %llu MB", Re_deviceInfo.localMemorySize / 1024 / 1024);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tRay Tracing: %ls", Re_deviceInfo.features.rayTracing ? L"yes" : L"no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tMesh Shading: %ls", Re_deviceInfo.features.rayTracing ? L"yes" : L"no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tUnified Memory: %ls", Re_deviceInfo.features.unifiedMemory ? L"yes" : L"no");
	
	Re_surface = Re_CreateSurface(E_screen);
	if (!Re_surface) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to create surface");
		return false;
	}
	
	Re_swapchain = Re_CreateSwapchain(Re_surface);
	if (!Re_swapchain) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to create swapchain");
		return false;
	}
	
	Re_LoadShaders();
	
	Re_InitPipelines();
	
	Re_contexts = calloc(1, sizeof(*Re_contexts));
	
	Re_contexts[0] = Re_CreateContext();
	
	//for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i) {

	//}
	
	return true;
}

void
Re_TermRender(void)
{
	Re_WaitIdle();

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
