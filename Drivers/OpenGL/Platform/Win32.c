#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>

#include <Windows.h>
#include <SetupAPI.h>
#include <rpc.h>

#include "../OpenGLDriver.h"

#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042

typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef BOOL (APIENTRY *PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC,
			const int *, const FLOAT *, UINT, int *, UINT *);

static HDC _dc;
static HGLRC _ctx;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
static PIXELFORMATDESCRIPTOR _pfd =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	PFD_MAIN_PLANE, 0, 0, 0, 0
};

static int _attr[20] =
{
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	0
};

bool
GL_InitContext(void)
{
	int fmt = 0;
	UINT count = 0;
	int *aptr = &_attr[8];
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

	if (CVAR_BOOL("Render_Multisampling")) {
		*aptr++ = WGL_SAMPLE_BUFFERS_ARB;
		*aptr++ = 1;
		*aptr++ = WGL_SAMPLES_ARB;
		*aptr++ = E_GetCVarI32("Render_Samples", 4)->i32;
	}

	_dc = GetDC((HWND)E_screen);

	fmt = ChoosePixelFormat(_dc, &_pfd);
	SetPixelFormat(_dc, fmt, &_pfd);
	
	_ctx = wglCreateContext(_dc);
	if (!_ctx)
		return false;

	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (wglChoosePixelFormatARB) {
		wglChoosePixelFormatARB(_dc, _attr, NULL, 1, &fmt, &count);
		SetPixelFormat(_dc, fmt, &_pfd);
		
		wglDeleteContext(_ctx);

		_ctx = wglCreateContext(_dc);
		if (!_ctx)
			return false;
	}

	if (!wglMakeCurrent(_dc, _ctx)) {
		wglDeleteContext(_ctx);
		return false;
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	
	gladLoadGL();
	
	return true;
}

void
GL_HardwareInfo(struct NeRenderDeviceInfo *info)
{
	SP_DEVINFO_DATA diData = { sizeof(diData) };
	SP_DRVINFO_DATA drvData = { sizeof(drvData) };
	SP_DRVINFO_DETAIL_DATA drvDetailData = { sizeof(drvDetailData) };
	HDEVINFO devInfo = NULL;
	DWORD len;
	GUID classGuid;
	CHAR *buff;
	char *ptr;

	SetupDiClassGuidsFromName(L"Display", &classGuid, 1, &len);
	devInfo = SetupDiGetClassDevs(&classGuid, NULL, NULL, DIGCF_PRESENT);
	if (!SetupDiEnumDeviceInfo(devInfo, 0, &diData)) {
		DWORD err = GetLastError();
		Sys_LogEntry("DDD", LOG_DEBUG, "GetLastError() = 0x%x", err);
		return;
	}

	SetupDiGetDeviceRegistryProperty(devInfo, &diData, SPDRP_HARDWAREID, NULL, NULL, 0, &len);
	buff = HeapAlloc(GetProcessHeap(), 0, len + 1);
	if (!buff)
		return;

	SetupDiGetDeviceRegistryProperty(devInfo, &diData, SPDRP_HARDWAREID, NULL, buff, len + 1, NULL);

	SetupDiGetDriverInfoDetail(devInfo, &diData, &drvData, &drvDetailData, sizeof(drvDetailData), &len);

	ptr = strstr(buff, "VEN_") + 4;
	info->hardwareInfo.vendorId = strtol(ptr, &ptr, 16);
	info->hardwareInfo.deviceId = strtol(ptr + 5, NULL, 16);
	info->hardwareInfo.driverVersion = (uint32_t)drvData.DriverVersion;
	info->localMemorySize = 0;

	HeapFree(GetProcessHeap(), 0, buff);
}

void
GL_EnableVerticalSync(bool enable)
{
	if (wglSwapIntervalEXT)
		wglSwapIntervalEXT(enable);
}

void
GL_SwapBuffers(void)
{
	SwapBuffers(_dc);
}

void *
GL_GetProcAddress(const char *name)
{
	return wglGetProcAddress(name);
}

void
GL_TermContext(void)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(_ctx);
}
