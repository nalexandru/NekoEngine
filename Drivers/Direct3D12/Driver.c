#include <stdio.h>
#include <stdlib.h>
#include <dxgidebug.h>

#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Render/Driver/Driver.h>

#include "D3D12Driver.h"

IDXGIFactory1 *D3D12_dxgiFactory = NULL;

static HMODULE _d3d12, _dxgiDebugModule;

static ID3D12Debug *_d3dDebug;
static IDXGIDebug *_dxgiDebug;
static IDXGIInfoQueue *_dxgiInfoQueue;
static ID3D12DeviceRemovedExtendedDataSettings *_dredSettings;

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct RenderDeviceInfo *);
static inline void _LogDXGIMessages(void);

static struct RenderDriver _drv =
{
	NE_RENDER_DRIVER_ID,
	NE_RENDER_DRIVER_API,
	L"Direct3D 12",
	_Init,
	_Term,
	_EnumerateDevices,
	D3D12_CreateDevice,
	D3D12_DestroyDevice
};

struct Array D3D12d_contexts;

#ifdef RENDER_DRIVER_BUILTIN
const struct RenderDriver *Re_LoadBuiltinDriver() { return &_drv; }
#else
#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT const struct RenderDriver *Re_LoadDriver(void) { return &_drv; }
#endif

static bool
_Init(void)
{
	_d3d12 = LoadLibrary(L"12on7\\d3d12.dll");
	if (!_d3d12) {
		_d3d12 = LoadLibrary(L"d3d12.dll");
		if (!_d3d12)
			return false;
	}

	if (E_GetCVarBln(L"D3D12Drv_Debug", true)->bln) {
		if (SUCCEEDED(D3D12GetDebugInterface(&IID_ID3D12Debug, &_d3dDebug)))
			_d3dDebug->lpVtbl->EnableDebugLayer(_d3dDebug);

		_dxgiDebugModule = LoadLibrary(L"dxgidebug.dll");
		if (_dxgiDebugModule) {
			HRESULT (*dxgi_GetDebugInterface)(REFIID, void **);
			dxgi_GetDebugInterface = (HRESULT (*)(REFIID, void **))GetProcAddress(_dxgiDebugModule, "DXGIGetDebugInterface");

			if (dxgi_GetDebugInterface) {
				dxgi_GetDebugInterface(&IID_IDXGIDebug, &_dxgiDebug);
				dxgi_GetDebugInterface(&IID_IDXGIInfoQueue, &_dxgiInfoQueue);
			}
		}

		if (SUCCEEDED(D3D12GetDebugInterface(&IID_ID3D12DeviceRemovedExtendedDataSettings, &_dredSettings))) {
			_dredSettings->lpVtbl->SetAutoBreadcrumbsEnablement(_dredSettings, D3D12_DRED_ENABLEMENT_FORCED_ON);
			_dredSettings->lpVtbl->SetPageFaultEnablement(_dredSettings, D3D12_DRED_ENABLEMENT_FORCED_ON);
			_dredSettings->lpVtbl->SetWatsonDumpEnablement(_dredSettings, D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
	}

	UUID features[] = { D3D12ExperimentalShaderModels };
	D3D12EnableExperimentalFeatures(1, features, NULL, NULL);

	if (FAILED(CreateDXGIFactory1(&IID_IDXGIFactory1, &D3D12_dxgiFactory)))
		return false;

	Rt_InitPtrArray(&D3D12d_contexts, (uint64_t)E_JobWorkerThreads() + 1, MH_RenderDriver);

	return true;
}

static void
_Term(void)
{
	Rt_TermArray(&D3D12d_contexts);

	IDXGIFactory1_Release(D3D12_dxgiFactory);

	if (_dxgiDebug) {
		IDXGIDebug_ReportLiveObjects(_dxgiDebug, DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		_LogDXGIMessages();

		IDXGIInfoQueue_Release(_dxgiInfoQueue);
		IDXGIDebug_Release(_dxgiDebug);
	}

	if (_d3dDebug)
		ID3D12Debug_Release(_d3dDebug);

	if (_dxgiDebugModule)
		FreeLibrary(_dxgiDebugModule);

	FreeLibrary(_d3d12);
}

static bool
_EnumerateDevices(uint32_t *count, struct RenderDeviceInfo *info)
{
	uint32_t i = 0;
	IDXGIAdapter1 *a;

	if (!*count || !info) {
		while (IDXGIFactory1_EnumAdapters1(D3D12_dxgiFactory, i++, &a) != DXGI_ERROR_NOT_FOUND)
			IDXGIAdapter1_Release(a);
		*count = i;
		return true;
	}

	while (i < *count && IDXGIFactory1_EnumAdapters1(D3D12_dxgiFactory, i++, &a) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		IDXGIAdapter1_GetDesc1(a, &desc);

		snprintf(info[i].deviceName, sizeof(info[i].deviceName), "%S", desc.Description);

		info[i].features.canPresent = true;
		info[i].features.astcTextureCompression = false;
		info[i].features.coherentMemory = true;

		info[i].features.unifiedMemory = false;

		info[i].features.discrete = !((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE);
		info[i].features.drawIndirectCount = true;
		info[i].features.bcTextureCompression = true;
		info[i].features.multiDrawIndirect = true;
		info[i].features.secondaryCommandBuffers = true;

		info[i].limits.maxTextureSize = 16384;

		info[i].localMemorySize = desc.DedicatedVideoMemory;
		info[i].private = (void *)(uint64_t)i;

		ID3D12Device5 *dev;
		HRESULT hr = D3D12CreateDevice((IUnknown *)a, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device5, &dev);

		IDXGIAdapter1_Release(a);

		if (FAILED(hr))
			continue;

		D3D12_FEATURE_DATA_D3D12_OPTIONS5 o5 = { 0 };
		ID3D12Device5_CheckFeatureSupport(dev, D3D12_FEATURE_D3D12_OPTIONS5, &o5, sizeof(o5));
		info[i].features.rayTracing = info[i].features.indirectRayTracing = o5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

		D3D12_FEATURE_DATA_D3D12_OPTIONS6 o6 = { 0 };
		ID3D12Device5_CheckFeatureSupport(dev, D3D12_FEATURE_D3D12_OPTIONS5, &o6, sizeof(o6));

		D3D12_FEATURE_DATA_D3D12_OPTIONS7 o7 = { 0 };
		ID3D12Device5_CheckFeatureSupport(dev, D3D12_FEATURE_D3D12_OPTIONS5, &o7, sizeof(o7));
		info[i].features.meshShading = o7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;

		ID3D12Device5_Release(dev);
	}

	return true;
}

static inline void
_LogDXGIMessages(void)
{
	static UINT64 nextId = 0;

	UINT64 count = IDXGIInfoQueue_GetNumStoredMessages(_dxgiInfoQueue, DXGI_DEBUG_ALL);

	for (UINT64 i = nextId; i < count; ++i) {
		SIZE_T length = 0;
		IDXGIInfoQueue_GetMessage(_dxgiInfoQueue, DXGI_DEBUG_ALL, i, NULL, &length);

		DXGI_INFO_QUEUE_MESSAGE *msg = (DXGI_INFO_QUEUE_MESSAGE *)Sys_Alloc(sizeof(wchar_t), length, MH_Transient);
		IDXGIInfoQueue_GetMessage(_dxgiInfoQueue, DXGI_DEBUG_ALL, i, msg, &length);

		uint8_t severity = LOG_DEBUG;
		switch (msg->Severity) {
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR: severity = LOG_CRITICAL; break;
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING: severity = LOG_WARNING; break;
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO: severity = LOG_INFORMATION; break;
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE: severity = LOG_DEBUG; break;
		}

		Sys_LogEntry(D3DDRV_MOD, severity, L"%S", msg->pDescription);
	}

	nextId = count;
}
