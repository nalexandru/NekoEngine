#include <assert.h>
#include <Windows.h>
#include <dxgidebug.h>
#include <VersionHelpers.h>
#include <d3d12sdklayers.h>

#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Shader.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <System/System.h>
#include <Engine/Resource.h>
#include <Scene/Components.h>

#include "Fence.h"
#include "Buffer.h"
#include "D3D12Render.h"

#define ENABLE_AFTERMATH	0

#if ENABLE_AFTERMATH
#	include <GFSDK_Aftermath.h>
#	include <GFSDK_Aftermath_GpuCrashDump.h>
#	pragma comment(lib, "GFSDK_Aftermath_Lib.x64.lib")
#endif

#define D3D12RMOD	L"D3D12Render"

static HMODULE _d3d12 = NULL;

struct RenderWorker *Re_Workers = NULL;
struct RenderWorker Re_MainThreadWorker = { 0 };
struct RenderFeatures Re_Features = { 0 };

struct GlobalRenderData Re_GlobalRenderData{};

struct RenderInfo Re_RenderInfo =
{
	{ L"Direct3D 12" },
	{ 0x0 },
	false
};

struct ResourceDestroyInfo
{
	ID3D12Resource *res;
	uint32_t frame;
};

static Array _pendingDestroy[RE_NUM_BUFFERS];

void _InitWorker(int worker, struct RenderWorker *w);
void _ResetWorker(struct RenderWorker *w);
void _TermWorker(struct RenderWorker *w);
static inline void _WaitForFence(void);

#include "RenderPath/RtRenderPath.h"
#include "RenderPath/FwdPlusRenderPath.h"

static RenderPath *_renderPath;

static ID3D12DeviceRemovedExtendedDataSettings *_dredSettings;
static ID3D12Debug *_d3dDebug;
static ID3D12DebugDevice2 *_debugDevice;
static IDXGIDebug *_dxgiDebug;
static IDXGIInfoQueue *_dxgiInfoQueue;
static HMODULE _dxgiDebugDLL;
static bool *_enableDebug = NULL;
static DWORD _workerKey = 0;
static UINT _syncInterval = 0;

static inline void _logDxgiMessages(void);

#if ENABLE_AFTERMATH
static GFSDK_Aftermath_ContextHandle _aftermathContext;
static void _gpuCrashDumpCallback(const void *crashDump, const uint32_t size, void *user);
static void _shaderDebugInfoCallback(const void *shaderDebugInfo, const uint32_t size, void *user);
static void _crashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void *user);
#endif

bool
Re_Init(void)
{
	_d3d12 = LoadLibrary(L"12on7\\d3d12.dll");
	if (!_d3d12) {
		_d3d12 = LoadLibrary(L"d3d12.dll");
		if (!_d3d12)
			return false;
	}

	_enableDebug = &E_GetCVarBln(L"D3D12_Debug", true)->bln;

#if ENABLE_AFTERMATH
	GFSDK_Aftermath_Result rc = GFSDK_Aftermath_EnableGpuCrashDumps(GFSDK_Aftermath_Version_API, GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX,
		GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default, _gpuCrashDumpCallback, _shaderDebugInfoCallback, _crashDumpDescriptionCallback, NULL);
	if (rc == GFSDK_Aftermath_Result_Success)
		Sys_LogEntry(D3D12RMOD, LOG_DEBUG, L"Aftermath enabled");
	else
		Sys_LogEntry(D3D12RMOD, LOG_DEBUG, L"Failed to enable Aftermath: %d", rc);
#else
	if (*_enableDebug) {
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&_d3dDebug))))
			_d3dDebug->EnableDebugLayer();
	
		_dxgiDebugDLL = LoadLibrary(L"dxgidebug.dll");
	
		HRESULT (*dxgi_GetDebugInterface)(REFIID, void **);
		dxgi_GetDebugInterface = (HRESULT (*)(REFIID, void **))GetProcAddress(_dxgiDebugDLL, "DXGIGetDebugInterface");
	
		if (dxgi_GetDebugInterface) {
			dxgi_GetDebugInterface(IID_PPV_ARGS(&_dxgiDebug));
			dxgi_GetDebugInterface(IID_PPV_ARGS(&_dxgiInfoQueue));
		}

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&_dredSettings)))) {
			_dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			_dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			_dredSettings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		}
	}
#endif

	UUID features[] = { D3D12ExperimentalShaderModels };
	D3D12EnableExperimentalFeatures(1, features, NULL, NULL);

	D3D12_InitDevice();

	if (*_enableDebug)
		Re_Device.dev->QueryInterface(IID_PPV_ARGS(&_debugDevice));

	if (_debugDevice && E_GetCVarBln(L"D3D12_Windows7Emulation", false)->bln) {
		D3D12_DEBUG_FEATURE feat = D3D12_DEBUG_FEATURE_EMULATE_WINDOWS7;
		_debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_FEATURE_FLAGS, &feat, sizeof(feat));
	}

#if ENABLE_AFTERMATH
	rc = GFSDK_Aftermath_DX12_Initialize(GFSDK_Aftermath_Version_API,
		GFSDK_Aftermath_FeatureFlags_Minimum |
	//	GFSDK_Aftermath_FeatureFlags_EnableMarkers |
		GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |
		GFSDK_Aftermath_FeatureFlags_CallStackCapturing |
		GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo,
		Re_Device.dev);
	if (rc == GFSDK_Aftermath_Result_Success)
		Sys_LogEntry(D3D12RMOD, LOG_DEBUG, L"Aftermath initialized for D3D12");
	else
		Sys_LogEntry(D3D12RMOD, LOG_DEBUG, L"Failed to initialize Aftermath: %d", rc);

	rc = GFSDK_Aftermath_DX12_CreateContextHandle(Re_Device.dev, &_aftermathContext);
	if (rc == GFSDK_Aftermath_Result_Success)
		Sys_LogEntry(D3D12RMOD, LOG_DEBUG, L"Aftermath device context created");
	else
		Sys_LogEntry(D3D12RMOD, LOG_DEBUG, L"Failed to create Aftermath device context: %d", rc);
#endif

	_workerKey = TlsAlloc();
	Re_Workers = (struct RenderWorker *)calloc(E_JobWorkerThreads(), sizeof(*Re_Workers));
	if (!Re_Workers)
		return false;

	_InitWorker(E_JobWorkerThreads(), &Re_MainThreadWorker);

	void **dispatchArgs = (void **)Sys_Alloc(sizeof(*dispatchArgs), E_JobWorkerThreads(), MH_Transient);
	for (int i = 0; i < E_JobWorkerThreads(); ++i)
		dispatchArgs[i] = &Re_Workers[i];

	E_DispatchJobs(E_JobWorkerThreads(), (JobProc)_InitWorker, dispatchArgs, NULL);

	for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
		D3DCHK(Re_Device.dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Re_Device.renderFence[i])));
		Re_Device.renderFence[i]->SetName(L"Frame fence");
		Re_Device.fenceValue[i] = 0;
		Rt_InitArray(&_pendingDestroy[i], 10, sizeof(struct ResourceDestroyInfo));
	}

	Re_Device.fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!Re_Device.fenceEvent)
		return false;

	if (!D3D12_InitStaging())
		return false;

	if (!D3D12_InitTextureHeap())
		return false;

	if (!D3D12_InitTransientHeap())
		return false;

	if (!D3D12_LoadShaders())
		return false;

	if (!D3D12_InitUI())
		return false;

	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(PREPARE_SCENE_DATA_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)D3D12_PrepareSceneData, 0);

	comp[0] = UI_CONTEXT_COMP;
	E_RegisterSystem(DRAW_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)D3D12_DrawUIContext, 0);

	_renderPath = new RtRenderPath();
	_renderPath->Init();

	_syncInterval = CVAR_BOOL(L"Render_VerticalSync");

	return true;
}

void
Re_Term(void)
{
	Re_WaitIdle();

	_renderPath->Term();
	delete _renderPath;

	D3D12_TermUI();

	D3D12_UnloadShaders();
	D3D12_TermTransientHeap();

	D3D12_TermTextureHeap();
	D3D12_TermStaging();

	for (size_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		for (size_t j = 0; j < _pendingDestroy[i].count; ++j) {
			ResourceDestroyInfo *rdi = (ResourceDestroyInfo *)Rt_ArrayGet(&_pendingDestroy[i], j);
			rdi->res->Release();
		}
		Rt_TermArray(&_pendingDestroy[i]);
	}

	for (int i = 0; i < E_JobWorkerThreads(); ++i)
		_TermWorker(&Re_Workers[i]);
	free(Re_Workers);

	_TermWorker(&Re_MainThreadWorker);

	TlsFree(_workerKey);

	for (int i = 0; i < RE_NUM_BUFFERS; ++i)
		Re_Device.renderFence[i]->Release();

	if (_debugDevice)
		_debugDevice->Release();

	D3D12_TermDevice();

	if (*_enableDebug) {
		if (_dxgiDebug) {
			_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			_logDxgiMessages();
	
			_dxgiInfoQueue->Release();
			_dxgiDebug->Release();
		}
	
		if (_d3dDebug)
			_d3dDebug->Release();
	
		FreeLibrary(_dxgiDebugDLL);
	}

	FreeLibrary(_d3d12);
}

void
Re_RenderFrame(void)
{
	{ // Begin frame
		SignalFence(&Re_UploadFence, Re_Device.transferQueue);

		D3D12_UpdateSceneData(Scn_ActiveScene);

		if (Re_Features.rayTracing) {
			D3DCHK(Re_MainThreadWorker.rtComputeList->Reset(Re_MainThreadWorker.computeAllocators[Re_Device.frame], NULL));
			D3D12_BuildBLAS(Re_MainThreadWorker.rtComputeList);
			D3D12_BuildTLAS(Re_MainThreadWorker.rtComputeList, Scn_ActiveScene);
			D3DCHK(Re_MainThreadWorker.rtComputeList->Close());

			D3DCHK(WaitForFenceGPU(&Re_UploadFence, Re_Device.computeQueue));
			ID3D12CommandList *lists[] = { Re_MainThreadWorker.computeList };
			Re_Device.computeQueue->ExecuteCommandLists(_countof(lists), lists);
			D3DCHK(SignalFence(&Re_ASFence, Re_Device.computeQueue));
			D3DCHK(WaitForFenceGPU(&Re_ASFence, Re_Device.graphicsQueue));
		}

		SignalFence(&Re_UploadFence, Re_Device.transferQueue);
	}
	
	{ // Render
		_renderPath->RenderScene(Scn_ActiveScene, Re_Device.targets[Re_Device.frame], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		D3D12_RenderUI(Scn_ActiveScene, Re_Device.targets[Re_Device.frame]);
	}

	{ // End frame
		D3DCHK(WaitForFenceGPU(&Re_UploadFence, Re_Device.graphicsQueue));

		if (Re_Device.swapChain) {
			D3DCHK(Re_MainThreadWorker.cmdList->Close());

			ID3D12CommandList *lists[] = { Re_MainThreadWorker.cmdList };
			Re_Device.graphicsQueue->ExecuteCommandLists(_countof(lists), lists);

			D3DCHK(Re_Device.swapChain->Present(_syncInterval, 0));
		} else {
			D3DCHK(Re_Device.graphicsQueueDownlevel->Present(Re_MainThreadWorker.cmdList, Re_Device.targets[Re_Device.frame],
				(HWND)E_Screen, _syncInterval ? D3D12_DOWNLEVEL_PRESENT_FLAG_WAIT_FOR_VBLANK : D3D12_DOWNLEVEL_PRESENT_FLAG_NONE));
		}

		++Re_Device.fenceValue[Re_Device.frame];
		D3DCHK(Re_Device.graphicsQueue->Signal(Re_Device.renderFence[Re_Device.frame], Re_Device.fenceValue[Re_Device.frame]));

		if (Re_Device.swapChain)
			Re_Device.frame = Re_Device.swapChain->GetCurrentBackBufferIndex();
		else
			Re_Device.frame = (Re_Device.frame + 1) % RE_NUM_BUFFERS;

		_WaitForFence();

		D3D12_ResetTransientHeap();
		D3D12_ResetUploadHeap();

		// Destroy pending resources
		for (size_t i = 0; i < _pendingDestroy[Re_Device.frame].count; ++i) {
			ResourceDestroyInfo *rdi = (ResourceDestroyInfo *)Rt_ArrayGet(&_pendingDestroy[Re_Device.frame], i);
			rdi->res->Release();
		}
		Rt_ClearArray(&_pendingDestroy[Re_Device.frame], false);

		_ResetWorker(&Re_MainThreadWorker);
	}
}

void
Re_ScreenResized(void)
{
	if (!Re_Device.dev)
		return;

	Re_WaitIdle();

	wchar_t *buff = (wchar_t *)Sys_Alloc(64, sizeof(wchar_t), MH_Transient);
	
	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i)
		Re_Device.targets[i]->Release();

	if (Re_Device.swapChain) {
		DXGI_SWAP_CHAIN_DESC1 scd;
		D3DCHK(Re_Device.swapChain->GetDesc1(&scd));
		D3DCHK(Re_Device.swapChain->ResizeBuffers(RE_NUM_BUFFERS, *E_ScreenWidth, *E_ScreenHeight, scd.Format, 0));
	}

	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		D3D12_RESOURCE_DESC bbd = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM,
			*E_ScreenWidth, *E_ScreenHeight, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		if (Re_Device.swapChain) {
			D3DCHK(Re_Device.swapChain->GetBuffer(i, IID_PPV_ARGS(&Re_Device.targets[i])));
		} else {
			D3DCHK(Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bbd, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&Re_Device.targets[i])));
		}

		swprintf_s(buff, 64, L"Render Target %d", i);
		Re_Device.targets[i]->SetName(buff);
	}
}

void
Re_WaitIdle(void)
{
	SignalFence(&Re_UploadFence, Re_Device.transferQueue);
	WaitForFenceCPU(&Re_UploadFence);

	for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
		D3DCHK(Re_Device.graphicsQueue->Signal(Re_Device.renderFence[i], ++Re_Device.fenceValue[i]));
		if (Re_Device.renderFence[i]->GetCompletedValue() < Re_Device.fenceValue[i]) {
			D3DCHK(Re_Device.renderFence[i]->SetEventOnCompletion(Re_Device.fenceValue[i], Re_Device.fenceEvent));
			WaitForSingleObject(Re_Device.fenceEvent, INFINITE);
		}
	}
	Re_Device.frame = 0;
}

struct RenderWorker *
D3D12_CurrentThreadWorker(void)
{
	return (struct RenderWorker *)TlsGetValue(_workerKey);
}

void
D3D12_DestroyResource(ID3D12Resource *res)
{
	ResourceDestroyInfo rdi = { res, Re_Device.frame };
	Rt_ArrayAdd(&_pendingDestroy[Re_Device.frame], &rdi);
}

void
D3D12_HandleDeviceRemoved(void)
{
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Device removed: 0x%x", Re_Device.dev->GetDeviceRemovedReason());

#if ENABLE_AFTERMATH
	Sleep(3000); // for NVIDIA Aftermath

	GFSDK_Aftermath_ContextData data;
	GFSDK_Aftermath_GetData(1, &_aftermathContext, &data);

	GFSDK_Aftermath_Device_Status status;
	GFSDK_Aftermath_GetDeviceStatus(&status);

	switch (status) {
	case GFSDK_Aftermath_Device_Status_Timeout: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Timeout"); break;
	case GFSDK_Aftermath_Device_Status_OutOfMemory: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Out of memory"); break;
	case GFSDK_Aftermath_Device_Status_PageFault: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Page fault"); break;
	case GFSDK_Aftermath_Device_Status_Stopped: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Device stopped"); break;
	case GFSDK_Aftermath_Device_Status_Reset: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Reset"); break;
	case GFSDK_Aftermath_Device_Status_DmaFault: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Dma Fault"); break;
	case GFSDK_Aftermath_Device_Status_Unknown: Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Status = Unknown"); break;
	}

	GFSDK_Aftermath_PageFaultInformation pfi;
	GFSDK_Aftermath_GetPageFaultInformation(&pfi);

	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"Page Fault Information:");
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\tPage Fault Occured: %s", pfi.bHasPageFaultOccured ? L"true" : L"false");
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\tFaulting VA: %llu", pfi.faultingGpuVA);
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\tResource: %d, %d, %d, %d, %d, 0x%x", pfi.resourceDesc.size,
		pfi.resourceDesc.width, pfi.resourceDesc.height, pfi.resourceDesc.depth, pfi.resourceDesc.mipLevels, pfi.resourceDesc.format);

	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\t\tIsBufferHeap: %s", pfi.resourceDesc.bIsBufferHeap ? L"true" : L"false");
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\t\tIsStaticTextureHeap: %s", pfi.resourceDesc.bIsStaticTextureHeap ? L"true" : L"false");
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\t\tIsRtvDsvTextureHeap: %s", pfi.resourceDesc.bIsRtvDsvTextureHeap ? L"true" : L"false");
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\t\tIsPlacedResource: %s", pfi.resourceDesc.bPlacedResource ? L"true" : L"false");
	Sys_LogEntry(D3D12RMOD, LOG_CRITICAL, L"\t\tWasDestroyed: %s", pfi.resourceDesc.bWasDestroyed ? L"true" : L"false");
#else
	ID3D12DeviceRemovedExtendedData *dred;
	Re_Device.dev->QueryInterface(IID_PPV_ARGS(&dred));

	D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT breadcrumb;
	D3D12_DRED_PAGE_FAULT_OUTPUT pageFault;

	dred->GetAutoBreadcrumbsOutput(&breadcrumb);
	dred->GetPageFaultAllocationOutput(&pageFault);
#endif

	E_Shutdown();
}

void
_InitWorker(int wid, struct RenderWorker *w)
{
	TlsSetValue(_workerKey, w);

	wchar_t *name = (wchar_t *)Sys_Alloc(sizeof(wchar_t), 64, MH_Transient);
	
	for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
		swprintf(name, 64, L"Worker %d Graphics Command Allocator %d", wid, i);
		D3DCHK(Re_Device.dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&w->allocators[i])));
		w->allocators[i]->SetName(name);

		swprintf(name, 64, L"Worker %d Copy Command Allocator %d", wid, i);
		D3DCHK(Re_Device.dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&w->copyAllocators[i])));
		w->copyAllocators[i]->SetName(name);

		swprintf(name, 64, L"Worker %d Compute Command Allocator %d", wid, i);
		D3DCHK(Re_Device.dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&w->computeAllocators[i])));
		w->computeAllocators[i]->SetName(name);
	}

	swprintf(name, 64, L"Worker %d Graphics Command List", wid);
	D3DCHK(Re_Device.dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, w->allocators[0], NULL, IID_PPV_ARGS(&w->cmdList)));
	w->cmdList->SetName(name);

	swprintf(name, 64, L"Worker %d Copy Command List", wid);
	D3DCHK(Re_Device.dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, w->copyAllocators[0], NULL, IID_PPV_ARGS(&w->copyList)));
	w->cmdList->SetName(name);

	swprintf(name, 64, L"Worker %d Compute Command List", wid);
	D3DCHK(Re_Device.dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, w->computeAllocators[0], NULL, IID_PPV_ARGS(&w->computeList)));
	w->cmdList->SetName(name);

	if (Re_Features.rayTracing)
		w->cmdList->QueryInterface(IID_PPV_ARGS(&w->rtCmdList));
		w->computeList->QueryInterface(IID_PPV_ARGS(&w->rtComputeList));

//#if ENABLE_AFTERMATH
//	GFSDK_Aftermath_result rc = GFSDK_Aftermath_DX12_CreateContextHandle(w->cmdList, )
//#endif

	w->copyList->Close();
	w->computeList->Close();
}

void
_ResetWorker(struct RenderWorker *w)
{
	D3DCHK(w->allocators[Re_Device.frame]->Reset());
	D3DCHK(w->copyAllocators[Re_Device.frame]->Reset());
	D3DCHK(w->computeAllocators[Re_Device.frame]->Reset());

	D3DCHK(w->cmdList->Reset(w->allocators[Re_Device.frame], NULL));
}

void
_TermWorker(struct RenderWorker *w)
{
	w->cmdList->Release();
	w->copyList->Release();
	w->computeList->Release();
	
	if (Re_Features.rayTracing) {
		w->rtCmdList->Release();
		w->rtComputeList->Release();
	}

	for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
		w->allocators[i]->Release();
		w->copyAllocators[i]->Release();
		w->computeAllocators[i]->Release();
	}
}

void
_WaitForFence(void)
{
	if (Re_Device.renderFence[Re_Device.frame]->GetCompletedValue() < Re_Device.fenceValue[Re_Device.frame]) {
		D3DCHK(Re_Device.renderFence[Re_Device.frame]->SetEventOnCompletion(Re_Device.fenceValue[Re_Device.frame], Re_Device.fenceEvent));
		WaitForSingleObject(Re_Device.fenceEvent, INFINITE);
	}
}

void
_logDxgiMessages(void)
{
	static UINT64 nextId = 0;

	UINT64 count = _dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);

	for (UINT64 i = nextId; i < count; ++i) {
		SIZE_T length{ 0 };
		_dxgiInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, NULL, &length);

		DXGI_INFO_QUEUE_MESSAGE *msg = (DXGI_INFO_QUEUE_MESSAGE *)Sys_Alloc(sizeof(wchar_t), length, MH_Transient);
		_dxgiInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, msg, &length);

		uint8_t severity = LOG_DEBUG;
		switch (msg->Severity) {
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR: severity = LOG_CRITICAL; break;
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING: severity = LOG_WARNING; break;
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO: severity = LOG_INFORMATION; break;
		case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE: severity = LOG_DEBUG; break;
		}

		Sys_LogEntry(D3D12RMOD, severity, L"%S", msg->pDescription);
	}

	nextId = count;
}

#if ENABLE_AFTERMATH

void
_gpuCrashDumpCallback(const void *crashDump, const uint32_t size, void *user)
{
	Sys_LogEntry(L"Aftermath", LOG_DEBUG, L"Gpu Crash");

	FILE *fp = fopen("gpucrash.dmp", "wb");
	fwrite(crashDump, size, 1, fp);
	fclose(fp);
}

void
_shaderDebugInfoCallback(const void *shaderDebugInfo, const uint32_t size, void *user)
{
	Sys_LogEntry(L"Aftermath", LOG_DEBUG, L"Shader Debug");
}

void
_crashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void *user)
{
	Sys_LogEntry(L"Aftermath", LOG_DEBUG, L"Gpu Crash Description");
}

#endif

