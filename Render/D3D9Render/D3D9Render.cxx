#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Scene/Components.h>
#include <System/Memory.h>
#include <System/Log.h>

#include "D3D9Render.h"

#define D3D9RMOD	L"D3D9Render"

#ifdef _XBOX
#	define D3DCREATE_MULTITHREADED	0
#endif

struct RenderInfo Re_RenderInfo = 
{
	{ L"Direct3D 9" },
	{ 0x0 },
	false
};
struct RenderFeatures Re_Features = { false, false, false };

struct RenderDevice Re_Device = { 0 };

IDirect3DVertexDeclaration9 *D3D9_VertexDeclaration;

static D3DPRESENT_PARAMETERS _pp = { 0 };

bool
Re_Init(void)
{
	UINT adapterIndex = D3DADAPTER_DEFAULT;
	HRESULT hr;
	D3DDISPLAYMODE dm;

	Re_Device.d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!Re_Device.d3d)
		return false;

	_pp.BackBufferWidth = *E_ScreenWidth;
	_pp.BackBufferHeight = *E_ScreenHeight;
	_pp.hDeviceWindow = (HWND)E_Screen;
	_pp.BackBufferCount = 2;
	_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	_pp.AutoDepthStencilFormat = D3DFMT_D24FS8;

#ifndef _XBOX

	UINT count = Re_Device.d3d->GetAdapterCount();
	if (count < 1) {
		return false;
	} else if (count > 1) {
		// select adapter
	}

	Re_Device.d3d->GetAdapterDisplayMode(adapterIndex, &dm);

	_pp.BackBufferFormat = dm.Format;
	_pp.Windowed = TRUE;
#else
	_pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	_pp.Windowed = FALSE;
#endif

	hr = Re_Device.d3d->CheckDeviceFormat(adapterIndex, D3DDEVTYPE_HAL, dm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32F_LOCKABLE);
	if (SUCCEEDED(hr)) {
		_pp.AutoDepthStencilFormat = D3DFMT_D32F_LOCKABLE;
	} else {
		hr = Re_Device.d3d->CheckDeviceFormat(adapterIndex, D3DDEVTYPE_HAL, dm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8);
		if (SUCCEEDED(hr)) {
			_pp.AutoDepthStencilFormat = D3DFMT_D24S8;
		} else {
			hr = Re_Device.d3d->CheckDeviceFormat(adapterIndex, D3DDEVTYPE_HAL, dm.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16);
			if (FAILED(hr))
				return false;

			_pp.AutoDepthStencilFormat = D3DFMT_D16;
		}
	}
	_pp.EnableAutoDepthStencil = TRUE;

	Re_Device.d3d->GetDeviceCaps(adapterIndex, D3DDEVTYPE_HAL, &Re_Device.caps);

	D3DADAPTER_IDENTIFIER9 identifier;
	Re_Device.d3d->GetAdapterIdentifier(adapterIndex, 0, &identifier);

	mbstowcs(Re_RenderInfo.device, identifier.Description, sizeof(Re_RenderInfo.device) / sizeof(wchar_t));

	/*if (CVAR_BOOL(L"Render_Multisampling")) {
		_pp.MultiSampleType = (D3DMULTISAMPLE_TYPE)CVAR_INT32(L"Render_Samples");

		if ((Re_Device.d3d->CheckDeviceMultiSampleType(adapterIndex, D3DDEVTYPE_HAL,
				_pp.BackBufferFormat, _pp.Windowed, _pp.MultiSampleType, NULL))) {
			Sys_LogEntry(D3D9RMOD, LOG_WARNING, L"%dx multisampling requested, but it isn't supported", _pp.MultiSampleType);
			_pp.MultiSampleType = D3DMULTISAMPLE_NONE;
		}
	}*/

	_pp.PresentationInterval = CVAR_BOOL(L"Render_VerticalSync") ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

	hr = Re_Device.d3d->CreateDevice(adapterIndex, D3DDEVTYPE_HAL, (HWND)E_Screen,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED, &_pp, &Re_Device.dev);
	if (FAILED(hr))
		return false;

	D3DVERTEXELEMENT9 *elements = (D3DVERTEXELEMENT9 *)Sys_Alloc(sizeof(*elements), 5, MH_Transient);
	memset(elements, 0x0, sizeof(*elements) * 5);

	elements[1].Method = elements[1].Method = elements[2].Method = elements[3].Method = D3DDECLMETHOD_DEFAULT;

	elements[0].Type = elements[1].Type = elements[2].Type = D3DDECLTYPE_FLOAT3;
	elements[3].Type = D3DDECLTYPE_FLOAT2;

	elements[0].Offset = offsetof(struct Vertex, x);
	elements[0].Usage = D3DDECLUSAGE_POSITION;

	elements[1].Offset = offsetof(struct Vertex, nx);
	elements[1].Usage = D3DDECLUSAGE_NORMAL;

	elements[2].Offset = offsetof(struct Vertex, tx);
	elements[2].Usage = D3DDECLUSAGE_TANGENT;

	elements[3].Offset = offsetof(struct Vertex, u);
	elements[3].Usage = D3DDECLUSAGE_TEXCOORD;

	elements[4].Stream = 0xFF; elements[4].Type = D3DDECLTYPE_UNUSED;

	if (FAILED(Re_Device.dev->CreateVertexDeclaration(elements, &D3D9_VertexDeclaration)))
		return false;

	Re_Device.dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	D3DVIEWPORT9 vp = { 0, 0, *E_ScreenWidth, *E_ScreenHeight, 0.f, 1.f };
	Re_Device.dev->SetViewport(&vp);

	const char *filterStr = E_GetCVarStr(L"Render_TextureFilter", "Nearest")->str;
	int32_t aniso = E_GetCVarI32(L"Render_TextureAnisotropy", 0)->i32;

	DWORD filter = D3DTEXF_POINT, mipFilter = D3DTEXF_POINT;
	if (!strncmp(filterStr, "Bilinear", strlen(filterStr))) {
		filter = D3DTEXF_LINEAR;
	} else if (!strncmp(filterStr, "Trilinear", strlen(filterStr))) {
		filter = D3DTEXF_LINEAR;
		mipFilter = D3DTEXF_LINEAR;
	} else if (!strncmp(filterStr, "Anisotropic", strlen(filterStr))) {
		filter = D3DTEXF_ANISOTROPIC;
		mipFilter = D3DTEXF_ANISOTROPIC;
	}

	for (DWORD i = 0; i < 7; ++i) {
		Re_Device.dev->SetSamplerState(i, D3DSAMP_MAGFILTER, filter);
		Re_Device.dev->SetSamplerState(i, D3DSAMP_MINFILTER, filter);
		Re_Device.dev->SetSamplerState(i, D3DSAMP_MIPFILTER, mipFilter);
		Re_Device.dev->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, aniso);
	}

	if (_pp.MultiSampleType != D3DMULTISAMPLE_NONE)
		Re_Device.dev->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);

	if (!D3D9_LoadShaders())
		return false;

	if (!D3D9_InitUI())
		return false;

	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(GET_DRAWABLES_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)D3D9_GetDrawables, 0);

	comp[0] = UI_CONTEXT_COMP;
	E_RegisterSystem(LOAD_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)D3D9_LoadUIContext, 0);
	E_RegisterSystem(DRAW_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)D3D9_DrawUIContext, 0);

	return true;
}

void
Re_Term(void)
{
	D3D9_TermUI();

	D3D9_UnloadShaders();

	Re_Device.dev->Release();
	Re_Device.d3d->Release();
}

void
Re_WaitIdle(void)
{
	//
}

void
Re_ScreenResized(void)
{
	if (!Re_Device.dev)
		return;

	_pp.BackBufferWidth = *E_ScreenWidth;
	_pp.BackBufferHeight = *E_ScreenHeight;

	Re_Device.dev->Reset(&_pp);
}

void
Re_RenderFrame(void)
{
	Re_Device.dev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(.8f, .2f, .2f, 1.f), 1.f, 0);
	Re_Device.dev->BeginScene();

	D3D9_RenderScene(Scn_ActiveScene);
	D3D9_RenderUI(Scn_ActiveScene);

	Re_Device.dev->EndScene();
	Re_Device.dev->Present(NULL, NULL, NULL, NULL);
}
