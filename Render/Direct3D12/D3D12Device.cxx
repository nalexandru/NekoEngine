#include <System/Log.h>
#include <System/System.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Engine/Engine.h>
#include <System/Memory.h>

#include "D3D12Render.h"

#define D3D12MOD	L"D3D12"

struct RenderDevice Re_Device = { 0 };

bool
D3D12_InitDevice(void)
{
	if (HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&Re_Device.factory)); FAILED(hr))
		return false;

	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_12_0;

	IDXGIAdapter1 *a;
	DXGI_ADAPTER_DESC1 aDesc = { 0 };
	int id = 0;

	while (Re_Device.factory->EnumAdapters1(id++, &a) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		a->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		ID3D12Device5 *dev;
		if (HRESULT hr = D3D12CreateDevice(a, level, IID_PPV_ARGS(&dev)); FAILED(hr))
			continue;

		if (desc.DedicatedVideoMemory > aDesc.DedicatedVideoMemory) {
			Re_Device.adapter = a;
			aDesc = desc;
		} else {
			a->Release();
		}

		dev->Release();
	}

	if (!Re_Device.adapter) {
		Sys_MessageBox(L"FATAL ERROR", L"No compatible D3D device found", MSG_ICON_ERROR);
		return false;
	}

	if (HRESULT hr = D3D12CreateDevice(Re_Device.adapter, level, IID_PPV_ARGS(&Re_Device.dev)); FAILED(hr)) {
		Sys_MessageBox(L"FATAL ERROR", L"Failed to create D3D device", MSG_ICON_ERROR);
		return false;
	}
	Re_Device.dev->SetName(L"Primary Device");

	memcpy(Re.info.device, aDesc.Description, wcslen(aDesc.Description) * sizeof(wchar_t));

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 o5{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS6 o6{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 o7{};

	Re_Device.dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &o5, sizeof(o5));
	Re_Device.dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &o6, sizeof(o6));
	Re_Device.dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &o7, sizeof(o7));

	Re.features.rayTracing = o5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
	Re.features.meshShading = o7.MeshShaderTier != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
	Re.features.variableRateShading = o6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
	Re.features.physicallyBased = true;

	swprintf(Re.info.name, sizeof(Re.info.name) / sizeof(wchar_t), L"Direct3D 12%ls", Re.features.rayTracing ? L" Ray Tracing" : L"");

	Sys_LogEntry(D3D12MOD, LOG_INFORMATION, L"GPU: %ls", aDesc.Description);
	Sys_LogEntry(D3D12MOD, LOG_INFORMATION, L"\tRay Tracing: %ls", Re.features.rayTracing ? L"true" : L"false");
	Sys_LogEntry(D3D12MOD, LOG_INFORMATION, L"\tMesh Shaders: %ls", Re.features.meshShading ? L"true" : L"false");
	Sys_LogEntry(D3D12MOD, LOG_INFORMATION, L"\tVariable Rate Shading: %ls", Re.features.variableRateShading ? L"true" : L"false");

	D3D12_COMMAND_QUEUE_DESC cqd =
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};

	if (HRESULT hr = Re_Device.dev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&Re_Device.graphicsQueue)); FAILED(hr)) {
		Sys_MessageBox(L"FATAL ERROR", L"Failed to create graphics command queue", MSG_ICON_ERROR);
		return false;
	}
	Re_Device.graphicsQueue->SetName(L"Graphics queue");
	Re_Device.graphicsQueue->QueryInterface(IID_PPV_ARGS(&Re_Device.graphicsQueueDownlevel));

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	if (HRESULT hr = Re_Device.dev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&Re_Device.computeQueue)); FAILED(hr)) {
		Sys_MessageBox(L"FATAL ERROR", L"Failed to create compute command queue", MSG_ICON_ERROR);
		return false;
	}
	Re_Device.computeQueue->SetName(L"Compute queue");

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	if (HRESULT hr = Re_Device.dev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&Re_Device.transferQueue)); FAILED(hr)) {
		Sys_MessageBox(L"FATAL ERROR", L"Failed to create transfer command queue", MSG_ICON_ERROR);
		return false;
	}
	Re_Device.transferQueue->SetName(L"Transfer queue");

	return D3D12_InitSwapchain();
}

bool
D3D12_InitSwapchain(void)
{
	DXGI_SWAP_CHAIN_DESC1 scd =
	{
		*E_ScreenWidth,
		*E_ScreenHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		FALSE,
		{
			1,
			0
		},
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		RE_NUM_BUFFERS,
		DXGI_SCALING_NONE,
		DXGI_SWAP_EFFECT_FLIP_DISCARD,
		DXGI_ALPHA_MODE_IGNORE,
	};

	if (FAILED(Re_Device.dev->QueryInterface(IID_PPV_ARGS(&Re_Device.downlevel)))) {
		IDXGIFactory2 *f2 = NULL;
		Re_Device.factory->QueryInterface(IID_PPV_ARGS(&f2));

		IDXGISwapChain1 *swapChain;
		if (Sys_UniversalWindows()) {
			if (HRESULT hr = f2->CreateSwapChainForCoreWindow(Re_Device.graphicsQueue, reinterpret_cast<IUnknown *>(E_Screen), &scd, NULL, &swapChain); FAILED(hr)) {
				Sys_MessageBox(L"FATAL ERROR", L"Failed to create swap chain", MSG_ICON_ERROR);
				return false;
			}
		} else {
			if (HRESULT hr = f2->CreateSwapChainForHwnd(Re_Device.graphicsQueue, (HWND)E_Screen, &scd, NULL, NULL, &swapChain); FAILED(hr)) {
				Sys_MessageBox(L"FATAL ERROR", L"Failed to create swap chain", MSG_ICON_ERROR);
				return false;
			}

			f2->MakeWindowAssociation((HWND)E_Screen, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);
		}

		if (HRESULT hr = swapChain->QueryInterface(IID_PPV_ARGS(&Re_Device.swapChain)); FAILED(hr)) {
			Sys_MessageBox(L"FATAL ERROR", L"IDXGISwapChain2 interface not found", MSG_ICON_ERROR);
			return false;
		}
		swapChain->Release();
	}

	Re_Device.frame = 0;
	if (Re_Device.swapChain)
		Re_Device.frame = Re_Device.swapChain->GetCurrentBackBufferIndex();

	wchar_t *buff = (wchar_t *)Sys_Alloc(64, sizeof(wchar_t), MH_Transient);
	
	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		D3D12_RESOURCE_DESC bbd = CD3DX12_RESOURCE_DESC::Tex2D(scd.Format, scd.Width, scd.Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
		D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		HRESULT hr;
		if (Re_Device.swapChain)
			hr = Re_Device.swapChain->GetBuffer(i, IID_PPV_ARGS(&Re_Device.targets[i]));
		else
			hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bbd, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(&Re_Device.targets[i]));

		if (FAILED(hr)) {
			Sys_MessageBox(L"FATAL ERROR", L"Failed to create render target", MSG_ICON_ERROR);
			return false;
		}

		swprintf_s(buff, 64, L"Render Target %d", i);
		Re_Device.targets[i]->SetName(buff);
	}

	return true;
}

void
D3D12_TermDevice(void)
{
	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i)
		Re_Device.targets[i]->Release();

	if (Re_Device.swapChain)
		Re_Device.swapChain->Release();

	if (Re_Device.graphicsQueue)
		Re_Device.graphicsQueue->Release();

	if (Re_Device.computeQueue)
		Re_Device.computeQueue->Release();

	if (Re_Device.transferQueue)
		Re_Device.transferQueue->Release();

	if (Re_Device.downlevel)
		Re_Device.downlevel->Release();

	if (Re_Device.dev)
		Re_Device.dev->Release();

	if (Re_Device.factory)
		Re_Device.factory->Release();
}
