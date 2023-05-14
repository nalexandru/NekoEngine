#include <stdio.h>
#include <stdlib.h>

#include <Engine/XR.h>
#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Render/Backend.h>

#include "D3D12Backend.h"

#include <dxgidebug.h>

#include <System/PlatformDetect.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")

const char *Re_backendName = "Direct3D 12";
struct NeArray D3D12_contexts{};
IDXGIFactory6 *D3D12_factory = nullptr;

static HMODULE f_dxgiDebugModule;
static IDXGIDebug *f_dxgiDebug;
static ID3D12Debug6 *f_d3dDebug;
static IDXGIInfoQueue *f_dxgiInfoQueue;
static ID3D12DeviceRemovedExtendedDataSettings1 *f_dredSettings;

typedef HRESULT (WINAPI *DXGIGETDEBUGINTERFACEPROC)(REFIID, void **);

// TODO: Windows 7 support. The following functions are not available on Windows 7:
typedef HRESULT (WINAPI *CREATEDXGIFACTORY2PROC)(UINT, REFIID, _COM_Outptr_ void **);	// dxgi.dll

bool
Re_InitBackend(void)
{
	/* TODO: Windows 7 support
	f_d3d12 = LoadLibrary(L"12on7\\d3d12.dll");
	if (!f_d3d12) {
		_d3d12 = LoadLibrary(L"d3d12.dll");
		if (!_d3d12)
			return false;
	}*/

#ifdef _DEBUG
	if (E_GetCVarBln("Direct3D12_EnableDebug", true)->bln) {
#else
	if (E_GetCVarBln("Direct3D12_EnableDebug", false)->bln) {
#endif
		if (HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&D3D12_factory)); FAILED(hr)) {
			Sys_LogEntry(D3D12BK_MOD, LOG_CRITICAL, "CreateDXGIFactory2 returned 0x%x", hr);
			return false;
		}
		if (HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&f_d3dDebug)); SUCCEEDED(hr)) {
			f_d3dDebug->EnableDebugLayer();
			f_d3dDebug->SetEnableGPUBasedValidation(TRUE);

			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&f_dredSettings)))) {
				f_dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				f_dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
				f_dredSettings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			}

			f_dxgiDebugModule = LoadLibrary(TEXT("dxgidebug"));
			if (f_dxgiDebugModule) {
				DXGIGETDEBUGINTERFACEPROC dxgiGetDebugInterface = (DXGIGETDEBUGINTERFACEPROC)GetProcAddress(f_dxgiDebugModule, "DXGIGetDebugInterface");

				if (dxgiGetDebugInterface) {
					dxgiGetDebugInterface(IID_PPV_ARGS(&f_dxgiDebug));
					dxgiGetDebugInterface(IID_PPV_ARGS(&f_dxgiInfoQueue));
				}
			}

			Sys_LogEntry(D3D12BK_MOD, LOG_WARNING, "Debug layer enabled");
			D3D12Bk_LogDXGIMessages();
		} else {
			Sys_LogEntry(D3D12BK_MOD, LOG_WARNING, "Failed to get ID3D12Debug6");
		}
	} else {
		if (HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&D3D12_factory)); FAILED(hr)) {
			Sys_LogEntry(D3D12BK_MOD, LOG_CRITICAL, "CreateDXGIFactory1 returned 0x%x", hr);
			return false;
		}
	}

	Rt_InitPtrArray(&D3D12_contexts, (size_t)E_JobWorkerThreads() + 1, MH_RenderBackend);

	return true;
}

void
Re_TermBackend(void)
{
	if (f_dxgiDebug) {
		f_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		D3D12Bk_LogDXGIMessages();

		f_dxgiInfoQueue->Release();
		f_dxgiDebug->Release();
	}

	if (f_d3dDebug)
		f_d3dDebug->Release();

	D3D12_factory->Release();

	if (f_dxgiDebugModule)
		FreeLibrary(f_dxgiDebugModule);
}

bool
Re_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	UINT id = 0;
	IDXGIAdapter4 *adapter;

	if (!*count || !info) {
		while (SUCCEEDED(D3D12_factory->EnumAdapterByGpuPreference(id++, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))) {
			DXGI_ADAPTER_DESC3 desc{};
			adapter->GetDesc3(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
				continue;

			ID3D12Device10 *dev;
			if (HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&dev)); FAILED(hr)) {
				adapter->Release();
				continue;
			}

			++(*count);
			dev->Release();
		}

		return true;
	}

	UINT ai = 0;
	info[ai].reserved = (void *)id;
	while (SUCCEEDED(D3D12_factory->EnumAdapterByGpuPreference(id++, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))) {
		DXGI_ADAPTER_DESC3 desc{};
		adapter->GetDesc3(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			continue;

		ID3D12Device10 *dev;
		if (HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&dev)); FAILED(hr)) {
			adapter->Release();
			continue;
		}

		snprintf(info[ai].deviceName, sizeof(info[ai].deviceName), "%ls", desc.Description);

		// required by D3D_FEATURE_LEVEL_12_2
		info[ai].features.meshShading = true;
		info[ai].features.rayTracing = true;

		info[ai].features.discrete = true; //props->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		info[ai].features.drawIndirectCount = false;//vk12Features->drawIndirectCount;
		info[ai].features.bcTextureCompression = true;//features->features.textureCompressionBC;
		info[ai].features.astcTextureCompression = false;//features->features.textureCompressionASTC_LDR;
		info[ai].features.multiDrawIndirect = true;//vk11Features->shaderDrawParameters && vk12Features->drawIndirectCount;
		info[ai].features.secondaryCommandBuffers = false;
		info[ai].features.directIO = true;

		info[ai].limits.maxTextureSize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		info[ai].limits.maxPushConstantsSize = 256;

		info[ai].localMemorySize = desc.DedicatedVideoMemory;
		info[ai].features.coherentMemory = true;


		info[ai].hardwareInfo.deviceId = desc.DeviceId;
		info[ai].hardwareInfo.vendorId = desc.VendorId;
		info[ai].hardwareInfo.driverVersion = desc.Revision;

		D3D12_FEATURE_DATA_ARCHITECTURE1 arch{};
		dev->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &arch, sizeof(arch));
		info[ai].features.unifiedMemory = arch.UMA;

		dev->Release();
		++ai;
	}

	return true;
}

struct NeSurface *
Re_CreateSurface(void *window)
{
	return (struct NeSurface *)1;
}

void
Re_DestroySurface(struct NeSurface *surface)
{
}

NeDirectIOHandle
Re_BkOpenFile(const char *path)
{
	return NULL;
}

void
Re_BkCloseFile(NeDirectIOHandle handle)
{
	//
}

void
D3D12Bk_LogDXGIMessages()
{
	if (!f_dxgiInfoQueue)
		return;

	static UINT64 nextId = 0;

	UINT64 count = f_dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);

	for (UINT64 i = nextId; i < count; ++i) {
		SIZE_T length{ 0 };
		f_dxgiInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, NULL, &length);

		DXGI_INFO_QUEUE_MESSAGE *msg = (DXGI_INFO_QUEUE_MESSAGE *)Sys_Alloc(sizeof(wchar_t), length, MH_Transient);
		f_dxgiInfoQueue->GetMessageW(DXGI_DEBUG_ALL, i, msg, &length);

		uint8_t severity = LOG_DEBUG;
		switch (msg->Severity) {
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR: severity = LOG_CRITICAL; break;
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING: severity = LOG_WARNING; break;
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO: severity = LOG_INFORMATION; break;
			case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE: severity = LOG_DEBUG; break;
		}

		Sys_LogEntry(D3D12BK_MOD, severity, "%s", msg->pDescription);
	}

	nextId = count;
}

/* NekoEngine
 *
 * D3D12Backend.cxx
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
