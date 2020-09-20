#include <Windows.h>
#include <crtdbg.h>

#include <Engine/Engine.h>
#include <System/System.h>

// NVIDIA Optimus
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
_declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

int APIENTRY
WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
#if 1
//	_crtBreakAlloc = 1923;
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#endif

	E_Init(__argc, __argv);

	return E_Run();
}
