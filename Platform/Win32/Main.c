#include "Win32Platform.h"
#include <crtdbg.h>
#include <stdio.h>

#include <Engine/Engine.h>
#include <System/System.h>

// NVIDIA Optimus
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

HINSTANCE Win32_instance;

static int
_AllocHook(int allocType, void *userData, size_t size,
	int blockType, long requestNumber,
	const unsigned char *filename, int lineNumber)
{
	return TRUE;
}

int APIENTRY
WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
#if 1
//	_crtBreakAlloc = 4335;
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
	_CrtSetAllocHook(_AllocHook);
#endif

	Win32_instance = hInst;

	if (!E_Init(__argc, __argv))
		return -1;

	return E_Run();
}
