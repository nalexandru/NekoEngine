#include <stdlib.h>
#include <string.h>

#include <System/Log.h>
#include <System/System.h>
#include <Engine/IO.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Driver.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Render/Pipeline.h>

#define RE_MOD L"Render"

struct RenderDevice *Re_Device;
struct RenderDeviceInfo Re_DeviceInfo = { 0 };
struct RenderDeviceProcs Re_DeviceProcs = { 0 };
struct RenderContextProcs Re_ContextProcs = { 0 };

void *Re_Surface = NULL;
void *Re_Swapchain = NULL;

static const struct RenderDriver *_drv = NULL;

#ifndef RENDER_DRIVER_BUILTIN
static void *_drvModule = NULL;
#endif

bool
Re_InitRender(void)
{
	uint32_t devCount = 0;
	struct RenderDeviceInfo *info = NULL, *selected = NULL;

#ifdef RENDER_DRIVER_BUILTIN
	_drv = Re_LoadBuiltinDriver();
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

	_drv = loadDriver();
#endif
	
	if (!_drv) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to load driver");
		return false;
	}
	
	if (_drv->identifier != NE_RENDER_DRIVER_ID) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"The library is not a valid driver");
		return false;
	}
	
	if (_drv->apiVersion != NE_RENDER_DRIVER_API) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Driver version mismatch");
		return false;
	}
	
	if (!_drv->Init()) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to initialize driver");
		return false;
	}
	
	if (!_drv->EnumerateDevices(&devCount, NULL)) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to enumerate devices");
		return false;
	}
	
	info = calloc(sizeof(*info), devCount);
	if (!_drv->EnumerateDevices(&devCount, info)) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to enumerate devices");
		return false;
	}
	
	uint64_t vramSize = 0;
	bool haveRt;
	for (uint32_t i = 0; i < devCount; ++i) {
		if (!info[i].features.canPresent || !info[i].features.discrete)
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
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"No suitable device found");
		return false;
	}
	
	memcpy(&Re_DeviceInfo, selected, sizeof(Re_DeviceInfo));
	free(info);
	
	Re_Device = _drv->CreateDevice(info, &Re_DeviceProcs, &Re_ContextProcs);
	if (!Re_Device) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to create device");
		return false;
	}
	
	// Device initialization
	
	if (!Re_InitDevice()) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to initialize device");
		return false;
	}
	
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"GPU: %hs", Re_DeviceInfo.deviceName);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tMemory: %llu MB", Re_DeviceInfo.localMemorySize / 1024 / 1024);
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tRay Tracing: %ls", Re_DeviceInfo.features.rayTracing ? L"yes" : L"no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tMesh Shading: %ls", Re_DeviceInfo.features.rayTracing ? L"yes" : L"no");
	Sys_LogEntry(RE_MOD, LOG_INFORMATION, L"\tUnified Memory: %ls", Re_DeviceInfo.features.unifiedMemory ? L"yes" : L"no");
	
	Re_Surface = Re_CreateSurface(Re_Device, E_Screen);
	if (!Re_Surface) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to create surface");
		return false;
	}
	
	Re_Swapchain = Re_CreateSwapchain(Re_Device, Re_Surface);
	if (!Re_Swapchain) {
		Sys_LogEntry(RE_MOD, LOG_CRITICAL, L"Failed to create swapchain");
		return false;
	}
	
	struct Stream stm;
	const char *cacheFile = E_GetCVarStr(L"Render_PipelineCachePath", "/UserData/Pipeline.cache")->str;
	if (E_FileStream(cacheFile, IO_READ, &stm)) {
		Re_LoadPipelineCache(&stm);
		E_CloseStream(&stm);
	}
	
	return true;
}

void
Re_TermRender(void)
{
	struct Stream stm;
	const char *cacheFile = E_GetCVarStr(L"Render_PipelineCachePath", "/UserData/Pipeline.cache")->str;
	if (E_FileStream(cacheFile, IO_WRITE, &stm)) {
		Re_SavePipelineCache(&stm);
		E_CloseStream(&stm);
	}
	
	Re_DestroySwapchain(Re_Device, Re_Swapchain);
	Re_DestroySurface(Re_Device, Re_Surface);
	Re_TermDevice();
	_drv->Term();

#ifndef RENDER_DRIVER_BUILTIN
	Sys_UnloadLibrary(_drvModule);
#endif
}

