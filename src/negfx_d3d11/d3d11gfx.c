/* NekoEngine
 *
 * d3d11gfx.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Direct3D 11 Graphics Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>

#include <system/log.h>
#include <system/config.h>
#include <graphics/graphics.h>

#include <gui.h>
#include <buffer.h>
#include <render.h>
#include <texture.h>
#include <d3d11gfx.h>

#define d3d11gfx_MODULE	"D3D11_Graphics"

ID3D11Device *d3d11_device = NULL;
ID3D11Device1 *d3d11_device1 = NULL;
ID3D11DeviceContext *d3d11_device_ctx = NULL;
ID3D11DeviceContext1 *d3d11_device_ctx1 = NULL;
IDXGISwapChain *swap_chain = NULL;
IDXGISwapChain1 *swap_chain1 = NULL;
ID3D11Texture2D *back_buffer_tex = NULL;

ID3D11Texture2D *color_tex = NULL;
ID3D11Texture2D *normal_tex = NULL;
ID3D11Texture2D *brightness_tex = NULL;
ID3D11Texture2D *depth_stencil_tex = NULL;

ID3D11RenderTargetView *color_rtv = NULL;
ID3D11RenderTargetView *normal_rtv = NULL;
ID3D11RenderTargetView *brightness_rtv = NULL;
ID3D11DepthStencilView *dsv = NULL;

static int _swap_interval = 1;
static D3D_FEATURE_LEVEL _feature_level;
static uint16_t _width, _height;
static float _clear_color[4] = { .8f, 0.f, .6f, 1.f },
	_clear_black[4] = { 0.f, 0.f, 0.f, 0.f };

ne_status d3d11gfx_init(bool);
void d3d11gfx_draw(void);
void d3d11gfx_swap_interval(int);
void d3d11gfx_screen_resized(uint16_t, uint16_t);
void d3d11gfx_wait_idle(void) { }
void d3d11gfx_release(void);

struct ne_gfx_module _d3d11gfx_module =
{
	NE_GFX_API_VER,
	d3d11gfx_init,
	d3d11gfx_draw,
	d3d11gfx_screen_resized,
	NULL, //gfx_sys_swap_interval,
	d3d11gfx_wait_idle,
	d3d11gfx_init_gui_drawable,
	d3d11gfx_free_gui_drawable,

	d3d11gfx_create_texture,
	d3d11gfx_upload_image,
	d3d11gfx_destroy_texture,

	d3d11gfx_register_font,
	d3d11gfx_unregister_font,

	d3d11gfx_create_buffer,
	d3d11gfx_map_buffer,
	d3d11gfx_unmap_buffer,
	d3d11gfx_copy_buffer,
	d3d11gfx_upload_buffer,
	d3d11gfx_flush_buffer,
	d3d11gfx_invalidate_buffer,
	d3d11gfx_destroy_buffer,

	d3d11gfx_init_material,
	d3d11gfx_release_material,

	d3d11gfx_init_drawable_mesh,
	d3d11gfx_release_drawable_mesh,
	d3d11gfx_add_mesh,

	d3d11gfx_release,

	true
};

const struct ne_gfx_module *
create_gfx_module(void)
{
	return &_d3d11gfx_module;
}

ne_status
_gfx_create_textures()
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = _width;
	desc.Height = _height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = 0;

	//desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	
	hr = ID3D11Device_CreateTexture2D(d3d11_device, &desc, NULL,
		&color_tex);
	if (hr != S_OK)
		return NE_FAIL;

	hr = ID3D11Device_CreateTexture2D(d3d11_device, &desc, NULL,
		&normal_tex);
	if (hr != S_OK)
		return NE_FAIL;

	hr = ID3D11Device_CreateTexture2D(d3d11_device, &desc, NULL,
		&brightness_tex);
	if (hr != S_OK)
		return NE_FAIL;

	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = ID3D11Device_CreateTexture2D(d3d11_device, &desc, NULL,
		&depth_stencil_tex);
	if (hr != S_OK)
		return NE_FAIL;

	D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
	ZeroMemory(&rtv_desc, sizeof(rtv_desc));
	rtv_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	hr = ID3D11Device_CreateRenderTargetView(d3d11_device, color_tex, NULL,
		&color_rtv);
	if (hr != S_OK)
		return NE_FAIL;

	hr = ID3D11Device_CreateRenderTargetView(d3d11_device, normal_tex, NULL,
		&normal_rtv);
	if (hr != S_OK)
		return NE_FAIL;

	hr = ID3D11Device_CreateRenderTargetView(d3d11_device, brightness_tex,
		NULL, &brightness_rtv);
	if (hr != S_OK)
		return NE_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	ZeroMemory(&dsv_desc, sizeof(dsv_desc));
	dsv_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

	hr = ID3D11Device_CreateDepthStencilView(d3d11_device, depth_stencil_tex,
		NULL, &dsv);
	if (hr != S_OK)
		return NE_FAIL;

	return NE_OK;
}

void
_gfx_destroy_textures(void)
{
	ID3D11Texture2D_Release(color_tex);
	ID3D11Texture2D_Release(normal_tex);
	ID3D11Texture2D_Release(brightness_tex);
	ID3D11Texture2D_Release(depth_stencil_tex);

	ID3D11RenderTargetView_Release(color_rtv);
	ID3D11RenderTargetView_Release(normal_rtv);
	ID3D11RenderTargetView_Release(brightness_rtv);
	ID3D11DepthStencilView_Release(dsv);
}

ne_status
d3d11gfx_init(bool debug)
{
	HWND hwnd = INVALID_HANDLE_VALUE;
	UINT flags = 0;
	HRESULT hr = 0;
	IDXGIFactory1 *dxgi_factory1 = NULL;
	IDXGIFactory2 *dxgi_factory2 = NULL;
	D3D_DRIVER_TYPE drv_type[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT drv_type_count = ARRAYSIZE(drv_type);

	D3D_FEATURE_LEVEL feat_lvl[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT feat_lvl_count = ARRAYSIZE(feat_lvl);

	log_entry(d3d11gfx_MODULE, LOG_INFORMATION, "Initializing...");

	hwnd = GetActiveWindow();
	_width = sys_config_get_int("width", 0);
	_height = sys_config_get_int("height", 0);

	if (debug)
		flags = D3D11_CREATE_DEVICE_DEBUG;

	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
		flags, feat_lvl, feat_lvl_count, D3D11_SDK_VERSION, &d3d11_device,
		&_feature_level, &d3d11_device_ctx);

	if (hr == E_INVALIDARG)
		// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1
		// so we need to retry without it
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
			flags, &feat_lvl[1], feat_lvl_count - 1, D3D11_SDK_VERSION,
			&d3d11_device, &_feature_level, &d3d11_device_ctx);

	if (FAILED(hr))
		return NE_GFX_CTX_CREATE_FAIL;

	
	// DXGI
	IDXGIDevice *dxgi_dev = NULL;
	hr = ID3D11Device_QueryInterface(d3d11_device, &IID_IDXGIDevice,
		&dxgi_dev);
	if (SUCCEEDED(hr)) {
		IDXGIAdapter *adapter = NULL;
		DXGI_ADAPTER_DESC desc;
		char name[128];

		hr = IDXGIDevice_GetAdapter(dxgi_dev, &adapter);
		if (SUCCEEDED(hr)) {
			hr = IDXGIAdapter_GetParent(adapter, &IID_IDXGIFactory1,
				&dxgi_factory1);

			IDXGIAdapter_GetDesc(adapter, &desc);
			log_entry(d3d11gfx_MODULE, LOG_INFORMATION, "Device: %ws",
				desc.Description);

			IDXGIAdapter_Release(adapter);
		}
		IDXGIDevice_Release(dxgi_dev);
	}
	if (FAILED(hr))
		return NE_GFX_CTX_CREATE_FAIL;

	{
		char *flvl_text = NULL;
		switch (_feature_level) {
		case D3D_FEATURE_LEVEL_10_0: flvl_text = "10.0"; break;
		case D3D_FEATURE_LEVEL_10_1: flvl_text = "10.1"; break;
		case D3D_FEATURE_LEVEL_11_0: flvl_text = "11.0"; break;
		case D3D_FEATURE_LEVEL_11_1: flvl_text = "11.1"; break;
		}
		log_entry(d3d11gfx_MODULE, LOG_INFORMATION,
			"Feature Level: Direct3D %s", flvl_text);
	}

	// Create swap chain
	hr = ID3D11Device_QueryInterface(d3d11_device, &IID_IDXGIFactory2,
		&dxgi_factory2);
	if (dxgi_factory2) {
		// DirectX 11.1 or later
		hr = ID3D11Device_QueryInterface(d3d11_device,
			&IID_ID3D11Device1, &d3d11_device1);
		if (SUCCEEDED(hr))
			hr = ID3D11DeviceContext_QueryInterface(d3d11_device_ctx,
				&IID_ID3D11DeviceContext1, &d3d11_device_ctx1);

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = _width;
		sd.Height = _height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = IDXGIFactory2_CreateSwapChainForHwnd(dxgi_factory2, d3d11_device,
			hwnd, &sd, NULL, NULL, &swap_chain1);
		if (SUCCEEDED(hr))
			hr = IDXGISwapChain1_QueryInterface(swap_chain1, &IID_IDXGISwapChain, &swap_chain);

		IDXGIFactory2_Release(dxgi_factory2);
	} else {
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = _width;
		sd.BufferDesc.Height = _height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = IDXGIFactory_CreateSwapChain(dxgi_factory1, d3d11_device,
			&sd, &swap_chain);
	}
	if (FAILED(hr))
		return NE_FAIL;

	IDXGIFactory_MakeWindowAssociation(dxgi_factory1, hwnd,
		DXGI_MWA_NO_ALT_ENTER);
	IDXGIFactory_Release(dxgi_factory1);

	/*hr = ID3D11Device_CreateRenderTargetView(d3d11_device, back_buff_tex, NULL, &d3d11_rtv);
	if (hr != S_OK)
		return NE_FAIL;*/

	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(vp));
	vp.Width = (FLOAT)_width;
	vp.Height = (FLOAT)_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ID3D11DeviceContext_RSSetViewports(d3d11_device_ctx, 1, &vp);

	return _gfx_create_textures();
}

static inline void
_gfx_z_prepass(void)
{
	ID3D11DeviceContext_OMSetRenderTargets(d3d11_device_ctx, 0, NULL, dsv);
	ID3D11DeviceContext_ClearDepthStencilView(d3d11_device_ctx, dsv,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	// get visible drawables from scene
	// render depth
}

static inline void
_gfx_lighting_pass(void)
{
	ID3D11RenderTargetView *views[3] =
	{
		color_rtv,
		normal_rtv,
		brightness_rtv
	};
	ID3D11DeviceContext_OMSetRenderTargets(d3d11_device_ctx, 3, views, dsv);

	ID3D11DeviceContext_ClearRenderTargetView(d3d11_device_ctx, color_rtv, _clear_color);
	ID3D11DeviceContext_ClearRenderTargetView(d3d11_device_ctx, normal_rtv, _clear_black);
	ID3D11DeviceContext_ClearRenderTargetView(d3d11_device_ctx, brightness_rtv, _clear_black);

	// draw scene
}

static inline void
_gfx_postprocess(void)
{
	//
}

void
d3d11gfx_draw(void)
{
	ID3D11Texture2D *back_buff_tex = NULL;

	_gfx_z_prepass();
	_gfx_lighting_pass();
	_gfx_postprocess();

	// Blit

	IDXGISwapChain_GetBuffer(swap_chain, 0, &IID_ID3D11Texture2D, &back_buff_tex);
	/*if (hr != S_OK)
		return NE_FAIL;*/
	
	/*ID3D11DeviceContext_ResolveSubresource(d3d11_device_ctx, back_buff_tex, 0,
		color_tex, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);*/

	ID3D11DeviceContext_ResolveSubresource(d3d11_device_ctx, back_buff_tex, 0,
		color_tex, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	//ID3D11DeviceContext_OMSetRenderTargets(d3d11_device_ctx, 1, &d3d11_rtv, NULL);

	/*glBlitNamedFramebuffer(_fbo, 0,
		0, 0, _width, _height,
		0, 0, _width, _height,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_NEAREST);*/

	IDXGISwapChain_Present(swap_chain, _swap_interval, 0);
}

void
d3d11gfx_swap_interval(int swi)
{
	_swap_interval = swi;
}

void
d3d11gfx_screen_resized(uint16_t width, uint16_t height)
{
	HRESULT hr;
	ID3D11Texture2D *back_buff_tex = NULL;
	D3D11_VIEWPORT vp;
	
	_width = width;
	_height = height;

	_gfx_destroy_textures();
	_gfx_create_textures();

	//ID3D11RenderTargetView_Release(d3d11_rtv);

	//hr = IDXGISwapChain_GetBuffer(swap_chain, 0, &IID_ID3D11Texture2D, &back_buff_tex);
	/*if (hr != S_OK)
		return NE_FAIL;*/
//
	//hr = ID3D11Device_CreateRenderTargetView(d3d11_device, back_buff_tex, NULL, &d3d11_rtv);
	/*if (hr != S_OK)
		return NE_FAIL;*/

	ZeroMemory(&vp, sizeof(vp));
	vp.Width = (FLOAT)_width;
	vp.Height = (FLOAT)_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ID3D11DeviceContext_RSSetViewports(d3d11_device_ctx, 1, &vp);
}

void
d3d11gfx_release(void)
{
	if (swap_chain)
		IDXGISwapChain_Release(swap_chain);
	if (swap_chain1)
		IDXGISwapChain1_Release(swap_chain1);

	if (d3d11_device)
		ID3D11Device_Release(d3d11_device);
	if (d3d11_device1)
		ID3D11Device1_Release(d3d11_device1);

	if (d3d11_device_ctx)
		ID3D11DeviceContext_Release(d3d11_device_ctx);
	if (d3d11_device_ctx1)
		ID3D11DeviceContext1_Release(d3d11_device_ctx);
}
