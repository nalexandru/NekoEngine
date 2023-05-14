#define INITGUID
#define COBJMACROS

#include <windows.h>
#include <setupapi.h>
#include <ddraw.h>

#include "../GLBackend.h"
#include "wglext.h"

#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>

#define GLW32_MOD	"GLWin32"

#define WGL_DRAW_TO_WINDOW_ARB			0x2001
#define WGL_SUPPORT_OPENGL_ARB			0x2010
#define WGL_DOUBLE_BUFFER_ARB			0x2011
#define WGL_PIXEL_TYPE_ARB				0x2013
#define WGL_COLOR_BITS_ARB				0x2014
#define WGL_DEPTH_BITS_ARB				0x2022
#define WGL_STENCIL_BITS_ARB			0x2023
#define WGL_FULL_ACCELERATION_ARB		0x2027
#define WGL_TYPE_RGBA_ARB				0x202B
#define WGL_SAMPLE_BUFFERS_ARB			0x2041
#define WGL_SAMPLES_ARB					0x2042

typedef HGLRC (APIENTRY *PFNWGLCREATECONTEXTPROC)(HDC);
typedef PROC (APIENTRY *PFNWGLGETPROCADDRESSPROC)(LPCSTR);
typedef BOOL (APIENTRY *PFNWGLDELETECONTEXTPROC)(HGLRC);
typedef BOOL (APIENTRY *PFNWGLMAKECURRENTPROC)(HDC, HGLRC);
typedef HRESULT (WINAPI *DIRECTDRAWCREATEPROC)(GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR *);

static HDC f_dc;
static HGLRC f_ctx;
static HMODULE f_opengl32;

static PFNWGLGETPROCADDRESSPROC f_wglGetProcAddress;
static PFNWGLMAKECURRENTPROC f_wglMakeCurrent;
static PFNWGLDELETECONTEXTPROC f_wglDeleteContext;
static PFNWGLSWAPINTERVALEXTPROC f_wglSwapIntervalEXT;
static PFNWGLCREATECONTEXTATTRIBSARBPROC f_wglCreateContextAttribsARB;

static PIXELFORMATDESCRIPTOR f_pfd =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	PFD_MAIN_PLANE, 0, 0, 0, 0
};

static int f_attr[20] =
{
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	0
};

static int f_ctxAttr[20] =
{
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	WGL_CONTEXT_FLAGS_ARB, 0,
	WGL_CONTEXT_OPENGL_NO_ERROR_ARB, FALSE,
	0
};

bool
GLBk_InitContext(void)
{
	int fmt = 0;
	UINT count = 0;
	int *aptr = &f_attr[8];
	PFNWGLCREATECONTEXTPROC wglCreateContext = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

	f_opengl32 = LoadLibrary(TEXT("opengl32"));
	if (!f_opengl32) {
		Sys_LogEntry(GLBK_MOD, LOG_CRITICAL, "Unable to load OpenGL32.dll !");
		return false;
	}

	wglCreateContext = (PFNWGLCREATECONTEXTPROC)GetProcAddress(f_opengl32, "wglCreateContext");
	f_wglMakeCurrent = (PFNWGLMAKECURRENTPROC)GetProcAddress(f_opengl32, "wglMakeCurrent");
	f_wglDeleteContext = (PFNWGLDELETECONTEXTPROC)GetProcAddress(f_opengl32, "wglDeleteContext");
	f_wglGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(f_opengl32, "wglGetProcAddress");

	if (CVAR_BOOL("Render_Multisampling")) {
		*aptr++ = WGL_SAMPLE_BUFFERS_ARB;
		*aptr++ = 1;
		*aptr++ = WGL_SAMPLES_ARB;
		*aptr++ = E_GetCVarI32("Render_Samples", 4)->i32;
	}

	{ // Set pixel format
		HWND dummy = CreateWindow(TEXT("STATIC"), TEXT("DummyWindow"), 0, 0, 0, 1, 1, HWND_DESKTOP, NULL, NULL, NULL);
		f_dc = GetDC(dummy);

		fmt = ChoosePixelFormat(f_dc, &f_pfd);
		SetPixelFormat(f_dc, fmt, &f_pfd);

		f_ctx = wglCreateContext(f_dc);
		if (!f_ctx)
			return false;

		if (!f_wglMakeCurrent(f_dc, f_ctx)) {
			f_wglDeleteContext(f_ctx);
			return false;
		}

		f_dc = GetDC((HWND)E_screen);

		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)f_wglGetProcAddress("wglChoosePixelFormatARB");
		if (wglChoosePixelFormatARB)
			wglChoosePixelFormatARB(f_dc, f_attr, NULL, 1, &fmt, &count);

		f_wglDeleteContext(f_ctx);
		DestroyWindow(dummy);
	}

	// FIXME: On Windows XP, the Radeon 9600 driver will not work with the result from wglChoosePixelFormatARB
	// is this because of loading the entry point from opengl32 instead of linking to it ?
	Sys_LogEntry("GLBK", LOG_DEBUG, "wgl: %d", fmt);
	fmt = ChoosePixelFormat(f_dc, &f_pfd);
	Sys_LogEntry("GLBK", LOG_DEBUG, "cpf: %d", fmt);
	SetPixelFormat(f_dc, fmt, &f_pfd);

	f_ctx = wglCreateContext(f_dc);
	if (!f_ctx)
		return false;

	if (!f_wglMakeCurrent(f_dc, f_ctx)) {
		f_wglDeleteContext(f_ctx);
		return false;
	}

	// Try to create an OpenGL 3.3 or newer (ideally 4.6) context
	f_wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)f_wglGetProcAddress("wglCreateContextAttribsARB");
	if (f_wglCreateContextAttribsARB) {
		if (E_GetCVarBln("OpenGL_CompatibilityContext", false)->bln)
			f_ctxAttr[1] = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		else if (E_GetCVarBln("OpenGL_ForwardCompatibleContext", true)->bln)
			f_ctxAttr[3] |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

		if (E_GetCVarBln("OpenGL_NoErrorContext", true)->bln)
			f_ctxAttr[5] = TRUE;

		if (CVAR_BOOL("Render_Debug")) {
			f_ctxAttr[5] = FALSE;
			f_ctxAttr[3] |= WGL_CONTEXT_DEBUG_BIT_ARB;
		}

		HGLRC ctx = f_wglCreateContextAttribsARB(f_dc, NULL, f_ctxAttr);
		if (ctx) {
			f_wglMakeCurrent(f_dc, NULL);
			f_wglDeleteContext(f_ctx);
			f_ctx = ctx;
		}
	}

	if (!f_wglMakeCurrent(f_dc, f_ctx)) {
		f_wglDeleteContext(f_ctx);
		return false;
	}

	f_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)f_wglGetProcAddress("wglSwapIntervalEXT");

	gladLoadGL();

	return true;
}

void *
GLBk_CreateShareContext(void)
{
	if (!f_wglCreateContextAttribsARB)
		return NULL;

	return f_wglCreateContextAttribsARB(f_dc, f_ctx, f_ctxAttr);
}

void
GLBk_DestroyShareContext(void *ctx)
{
	if (ctx)
		f_wglDeleteContext(ctx);
}

void
GLBk_MakeCurrent(void *ctx)
{
	f_wglMakeCurrent(f_dc, (HGLRC)ctx);
}

void
GLBk_EnableVerticalSync(bool enable)
{
	if (f_wglSwapIntervalEXT)
		f_wglSwapIntervalEXT(enable);
}

void
GLBk_SwapBuffers(void)
{
	SwapBuffers(f_dc);
}

void
GLBk_HardwareInfo(struct NeRenderDeviceInfo *info)
{
	SP_DEVINFO_DATA diData = { sizeof(diData) };
	SP_DRVINFO_DATA drvData = { sizeof(drvData) };
	SP_DRVINFO_DETAIL_DATA drvDetailData = { sizeof(drvDetailData) };
	HDEVINFO devInfo = NULL;
	DWORD len;
	GUID classGuid;
	WCHAR *buff;
	wchar_t *ptr;

	SetupDiClassGuidsFromName(TEXT("Display"), &classGuid, 1, &len);
	devInfo = SetupDiGetClassDevs(&classGuid, NULL, NULL, DIGCF_PRESENT);
	if (!SetupDiEnumDeviceInfo(devInfo, 0, &diData)) {
		DWORD err = GetLastError();
		Sys_LogEntry(GLW32_MOD, LOG_DEBUG, "GetLastError() = 0x%x", err);
		return;
	}

	SetupDiGetDeviceRegistryProperty(devInfo, &diData, SPDRP_HARDWAREID, NULL, NULL, 0, &len);
	buff = HeapAlloc(GetProcessHeap(), 0, sizeof(WCHAR) * (len + 1));
	if (!buff)
		return;

	SetupDiGetDeviceRegistryProperty(devInfo, &diData, SPDRP_HARDWAREID, NULL, (PBYTE)buff, len + 1, NULL);
	SetupDiGetDriverInfoDetail(devInfo, &diData, &drvData, &drvDetailData, sizeof(drvDetailData), &len);

	ptr = wcsstr(buff, TEXT("VEN_")) + 4;
	info->hardwareInfo.vendorId = wcstol(ptr, &ptr, 16);
	info->hardwareInfo.deviceId = wcstol(ptr + 5, NULL, 16);
	info->hardwareInfo.driverVersion = (uint32_t)drvData.DriverVersion;
	info->localMemorySize = 0;

	HeapFree(GetProcessHeap(), 0, buff);

	// Use DirectDraw to get video memory info
	HMODULE ddrawLib = LoadLibrary(TEXT("ddraw"));
	if (ddrawLib) {
		LPDIRECTDRAW ddraw = NULL;
		DIRECTDRAWCREATEPROC ddCreate = (DIRECTDRAWCREATEPROC)GetProcAddress(ddrawLib, "DirectDrawCreate");
		if (ddCreate) {
			ddCreate(NULL, &ddraw, NULL);

			LPDIRECTDRAW7 ddraw7 = NULL;
			if (SUCCEEDED(IDirectDraw_QueryInterface(ddraw, &IID_IDirectDraw7, (VOID **)&ddraw7))) {
				DWORD vram;
				DDSCAPS2 ddscaps = { .dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM };
				
				IDirectDraw7_GetAvailableVidMem(ddraw7, &ddscaps, &vram, NULL);
				IDirectDraw7_Release(ddraw7);

				info->localMemorySize = vram;
			}
		}

		FreeLibrary(ddrawLib);
	}

	if (!info->localMemorySize)
		info->localMemorySize = 1;	// the Engine will refuse the device if it has no VRAM.
}

void
GLBk_TermContext(void)
{
	f_wglMakeCurrent(NULL, NULL);
	f_wglDeleteContext(f_ctx);
}
