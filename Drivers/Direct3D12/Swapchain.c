#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "D3D12Driver.h"

static inline bool _Create(struct RenderDevice *dev, struct Swapchain *sw);

static IDXGIFactory2 *_factory = NULL;

struct Swapchain *
D3D12_CreateSwapchain(struct RenderDevice *dev, void *surface, bool verticalSync)
{
	if (FAILED(IDXGIFactory1_QueryInterface(D3D12_dxgiFactory, &IID_IDXGIFactory2, &_factory)))
		return NULL;

	struct Swapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);

	sw->surface = surface;

	sw->desc.Stereo = FALSE;
	sw->desc.SampleDesc.Count = 1;
	sw->desc.SampleDesc.Quality = 0;
	sw->desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sw->desc.BufferCount = RE_NUM_FRAMES;
	sw->desc.Scaling = DXGI_SCALING_NONE;
	sw->desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	sw->desc.Flags = 0;

	sw->desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	/*VkSurfaceFormatKHR *formats = Sys_Alloc(sizeof(*formats), count, MH_Transient);
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev->physDev, surface, &count, formats);
	for (uint32_t i = 0; i < count; ++i) {
		if (formats[i].format == VK_FORMAT_R16G16B16A16_SFLOAT) {
			sw->surfaceFormat = formats[i];
			break;
		}

		if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
			sw->surfaceFormat = formats[i];
		else if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && sw->surfaceFormat.format != VK_FORMAT_R8G8B8A8_UNORM)
			sw->surfaceFormat = formats[i];
	}*/


	sw->desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
/*	if (!E_GetCVarBln(L"Render_VerticalSync", false)->bln) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev->physDev, surface, &count, NULL);

		VkPresentModeKHR *pm = Sys_Alloc(sizeof(*pm), count, MH_Transient);
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev->physDev, surface, &count, pm);

		for (uint32_t i = 0; i < count; ++i) {
			if (pm[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				sw->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if (sw->presentMode != VK_PRESENT_MODE_MAILBOX_KHR && pm[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				sw->presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}*/

	sw->presentInterval = verticalSync ? 1 : 0;

	if (!_Create(dev, sw))
		goto error;

	return sw;

error:
	Sys_Free(sw);

	return NULL;
}

void
D3D12_DestroySwapchain(struct RenderDevice *dev, struct Swapchain *sw)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		ID3D12Resource_Release(sw->buffers[i]);

	IDXGISwapChain3_Release(sw->sw);
	Sys_Free(sw);
}

void *
D3D12_AcquireNextImage(struct RenderDevice *dev, struct Swapchain *sw)
{
	if (ID3D12Fence_GetCompletedValue(dev->renderFence[Re_frameId]) < dev->fenceValue[Re_frameId]) {
		ID3D12Fence_SetEventOnCompletion(dev->renderFence[Re_frameId], dev->fenceValue[Re_frameId], dev->fenceEvent);
		WaitForSingleObject(dev->fenceEvent, INFINITE);
	}

	return sw->buffers[Re_frameId];
}

bool
D3D12_Present(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *sw, void *image)
{
	++dev->fenceValue[Re_frameId];

	IDXGISwapChain3_Present(sw->sw, sw->presentInterval, 0);

	ID3D12CommandQueue_Signal(dev->graphicsQueue, dev->renderFence[Re_frameId], dev->fenceValue[Re_frameId]);


/*	dev->frameValues[Re_frameId] = ++dev->semaphoreValue;

	uint64_t waitValues[] = { 0 };
	uint64_t signalValues[] = { dev->frameValues[Re_frameId], 0 };
	VkSemaphore wait[] = { sw->frameStart[Re_frameId] };
	VkSemaphore signal[] = { dev->frameSemaphore, sw->frameEnd[Re_frameId] };
	VkPipelineStageFlags waitMasks[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

	VkTimelineSemaphoreSubmitInfo timelineInfo =
	{
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.waitSemaphoreValueCount = 1,
		.pWaitSemaphoreValues = waitValues,
		.signalSemaphoreValueCount = 2,
		.pSignalSemaphoreValues = signalValues
	};
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineInfo,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait,
		.pWaitDstStageMask = waitMasks,
		.commandBufferCount = (uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count,
		.pCommandBuffers = (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data,
		.signalSemaphoreCount = 2,
		.pSignalSemaphores = signal
	};
	vkQueueSubmit(dev->graphicsQueue, 1, &si, VK_NULL_HANDLE);

	uint32_t imageId = (uint32_t)(uint64_t)image;

	VkPresentInfoKHR pi =
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &sw->frameEnd[Re_frameId],
		.swapchainCount = 1,
		.pSwapchains = &sw->sw,
		.pImageIndices = &imageId
	};
	VkResult rc = vkQueuePresentKHR(dev->graphicsQueue, &pi);

	switch (rc) {
	case VK_SUCCESS: return true;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		return _Create(dev->dev, dev->physDev, sw);
	default: return false;
	}

	return rc == VK_SUCCESS;*/
	return false;
}

enum TextureFormat
D3D12_SwapchainFormat(struct Swapchain *sw)
{
	return DXGIToNeTextureFormat(sw->desc.Format);
}

struct Texture *
D3D12_SwapchainTexture(struct Swapchain *sw, void *image)
{
	struct Texture *t = Sys_Alloc(sizeof(*t), 1, MH_Transient);
	t->res = image;
	return t;
}

void
D3D12_ScreenResized(struct RenderDevice *dev, struct Swapchain *sw)
{
	if (!_Create(dev, sw)) {
		Sys_LogEntry(D3DDRV_MOD, LOG_CRITICAL, L"Failed to resize swapchain");
		E_Shutdown();
	}
}

static inline bool
_Create(struct RenderDevice *dev, struct Swapchain *sw)
{
	D3D12_WaitIdle(dev);

	if (sw->sw)
		IDXGISwapChain3_Release(sw->sw);

	IDXGISwapChain1 *sw1 = NULL;
	HRESULT hr = S_FALSE;
	if (sw->surface->hWnd)
		hr = IDXGIFactory2_CreateSwapChainForHwnd(_factory, (IUnknown *)dev->graphicsQueue, sw->surface->hWnd, &sw->desc, NULL, NULL, &sw1);
	else if (sw->surface->coreWindow)
		hr = IDXGIFactory2_CreateSwapChainForCoreWindow(_factory, (IUnknown *)dev->graphicsQueue, sw->surface->coreWindow, &sw->desc, NULL, &sw1);

	if (FAILED(hr) || !sw1)
		return false;

	if (sw->surface->hWnd)
		IDXGIFactory2_MakeWindowAssociation(_factory, sw->surface->hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

	hr = IDXGISwapChain1_QueryInterface(sw1, &IID_IDXGISwapChain3, &sw->sw);
	IDXGISwapChain1_Release(sw1);

	if (FAILED(hr))
		return false;

	Re_frameId = IDXGISwapChain3_GetCurrentBackBufferIndex(sw->sw);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (sw->buffers[i])
			ID3D12Resource_Release(sw->buffers[i]);

		if (FAILED(IDXGISwapChain3_GetBuffer(sw->sw, i, &IID_ID3D12Resource, &sw->buffers[i])))
			return false;

		ID3D12Resource_SetName(sw->buffers[i], L"Render Target");
	}

	return true;
}
