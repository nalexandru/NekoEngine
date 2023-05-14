#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "D3D12Backend.h"

#define D3D12_SWMOD	"D3D12Swapchain"

struct NeSwapchain *
Re_CreateSwapchain(struct NeSurface *surface, bool verticalSync)
{
	struct NeSwapchain *sw = (NeSwapchain *)Sys_Alloc(1, sizeof(*sw), MH_RenderBackend);

	sw->desc.Width = *E_screenWidth;
	sw->desc.Height = *E_screenHeight;
	sw->desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sw->desc.Stereo = FALSE;
	sw->desc.SampleDesc.Count = 1;
	sw->desc.SampleDesc.Quality = 0;
	sw->desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sw->desc.BufferCount = RE_NUM_FRAMES;
	sw->desc.Scaling = DXGI_SCALING_NONE;
	sw->desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sw->desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	// TODO: UWP support
	IDXGISwapChain1 *swapChain;
	if (FAILED(D3D12_factory->CreateSwapChainForHwnd(Re_device->direct, (HWND)E_screen, &sw->desc,
														NULL, NULL, &swapChain)))
		return NULL;

	if (FAILED(swapChain->QueryInterface(IID_PPV_ARGS(&sw->chain)))) {
		Sys_MessageBox("FATAL ERROR", "IDXGISwapChain3 interface not found", MSG_ICON_ERROR);
		swapChain->Release();
		return NULL;
	}
	swapChain->Release();

	D3D12_factory->MakeWindowAssociation((HWND)E_screen, 0);
	Re_frameId = sw->chain->GetCurrentBackBufferIndex();

	wchar_t *buff = (wchar_t *)Sys_Alloc(64, sizeof(wchar_t), MH_Transient);

	for (uint32_t i = 0; i < sw->desc.BufferCount; ++i) {
		if (FAILED(sw->chain->GetBuffer(i, IID_PPV_ARGS(&sw->targets[i])))) {
			Sys_MessageBox("FATAL ERROR", "Failed to create render target", MSG_ICON_ERROR);
			return NULL;
		}

		swprintf_s(buff, 64, L"Render Target %d", i);
		sw->targets[i]->SetName(buff);
	}

	return sw;
}

void
Re_DestroySwapchain(struct NeSwapchain *sw)
{
	for (uint32_t i = 0; i < sw->desc.BufferCount; ++i)
		sw->targets[i]->Release();

	sw->chain->Release();

/*	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		vkDestroySemaphore(Re_device->dev, sw->frameEnd[i], Vkd_allocCb);
		vkDestroySemaphore(Re_device->dev, sw->frameStart[i], Vkd_allocCb);
	}*/

	Sys_Free(sw);
}

void *
Re_AcquireNextImage(struct NeSwapchain *sw)
{
	/*VkSemaphoreWaitInfo waitInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &Re_device->frameSemaphore,
			.pValues = &Re_device->frameValues[Re_frameId]
		};
	vkWaitSemaphores(Re_device->dev, &waitInfo, UINT64_MAX);

	uint32_t imageId;
	VkResult rc = vkAcquireNextImageKHR(Re_device->dev, sw->sw, UINT64_MAX, sw->frameStart[Re_frameId], VK_NULL_HANDLE, &imageId);
	if (rc != VK_SUCCESS) {
		switch (rc) {
			case VK_SUBOPTIMAL_KHR:
			case VK_ERROR_OUT_OF_DATE_KHR:
				Create(Re_device->dev, Re_device->physDev, sw);
				return RE_INVALID_IMAGE;
			default: return RE_INVALID_IMAGE;
		}
	}

	if (!Re_deviceInfo.features.coherentMemory)
		Vkd_CommitStagingArea(Re_device, sw->frameStart[Re_frameId]);

	vkResetDescriptorPool(Re_device->dev, Re_device->iaDescriptorPool[Re_frameId], 0);

	return (void *)(uint64_t)imageId;*/

	/*
	 * SignalFence(&Re_UploadFence, Re_Device.transferQueue);

		D3D12_UpdateSceneData(Scn_ActiveScene);

		if (Re.features.rayTracing) {
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
	 */

	return NULL;
}

bool
Re_Present(struct NeSwapchain *sw, void *image, struct NeSemaphore *waitSemaphore)
{
	// execute commands

	//D3DCHK(WaitForFenceGPU(&Re_UploadFence, Re_Device.graphicsQueue));

	return SUCCEEDED(sw->chain->Present(sw->presentInterval, 0));

	/*++Re_Device.fenceValue[Re_Device.frame];
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

	_ResetWorker(&Re_MainThreadWorker);*/
}

enum NeTextureFormat
Re_SwapchainFormat(struct NeSwapchain *sw)
{
	return DXGIToNeTextureFormat(sw->desc.Format);
}

void
Re_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
	desc->format = DXGIToNeTextureFormat(sw->desc.Format);
	//desc->usage = sw->imageUsage;
}

void
Re_SwapchainTextureDesc(struct NeSwapchain *sw, struct NeTextureDesc *desc)
{
	desc->width = *E_screenWidth;
	desc->height = *E_screenHeight;
	desc->depth = 1;
	desc->format = DXGIToNeTextureFormat(sw->desc.Format);
//	desc->usage = sw->imageUsage;
	desc->type = TT_2D;
	desc->arrayLayers = 1;
	desc->mipLevels = 1;
	desc->gpuOptimalTiling = true;
	desc->memoryType = MT_GPU_LOCAL;
}

struct NeTexture *
Re_SwapchainTexture(struct NeSwapchain *sw, void *image)
{
	uint32_t id = (uint32_t)(uint64_t)image;

	NeTexture *t = (NeTexture *)Sys_Alloc(sizeof(*t), 1, MH_Transient);
	t->res = sw->targets[id];

	return t;
}

void
Re_ScreenResized(struct NeSwapchain *sw)
{
	if (const HRESULT hr = sw->chain->ResizeBuffers(sw->desc.BufferCount, *E_screenWidth, *E_screenHeight,
												sw->desc.Format, sw->desc.Flags); FAILED(hr)) {
		Sys_LogEntry(D3D12BK_MOD, LOG_CRITICAL, "Failed to resize swapchain: 0x%x", hr);
		E_Shutdown();
	}

	Re_frameId = sw->chain->GetCurrentBackBufferIndex();

	wchar_t *buff = (wchar_t *)Sys_Alloc(64, sizeof(wchar_t), MH_Transient);

	for (uint32_t i = 0; i < sw->desc.BufferCount; ++i) {
		if (FAILED(sw->chain->GetBuffer(i, IID_PPV_ARGS(&sw->targets[i])))) {
			Sys_MessageBox("FATAL ERROR", "Failed to create render target", MSG_ICON_ERROR);
			E_Shutdown();
		}

		swprintf_s(buff, 64, L"Render Target %d", i);
		sw->targets[i]->SetName(buff);
	}
}

/* NekoEngine
 *
 * D3D12Swapchain.cxx
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
