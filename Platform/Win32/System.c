#include <stdio.h>
#include <Windows.h>

#include <System/System.h>
#include <Engine/Engine.h>

#include "Win32Platform.h"

static int _numCpus = 0;
static HANDLE h_stdout;
static HANDLE h_stderr;
static WORD _colors[4] =
{
	13, 7, 14, 12
};
static WORD _defaultColor;

bool
Sys_InitDbgOut(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!IsDebuggerPresent() && Sys_MachineType() == MT_PC) {
		FreeConsole();
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

		(void)freopen("CON", "w", stdout);
		(void)freopen("CON", "w", stderr);

		SetConsoleTitleA("Debug Console");
	}

	h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	h_stderr = GetStdHandle(STD_ERROR_HANDLE);

	GetConsoleScreenBufferInfo(h_stdout, &csbi);
	_defaultColor = csbi.wAttributes;

	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	SetConsoleTextAttribute(h_stdout, _colors[color]);

	if (IsDebuggerPresent()) {
		wchar_t buff[4096];
		swprintf(buff, 4096, L"[%s][%s]: %s\n", module, severity, text);
		OutputDebugString(buff);
	} else {
		fwprintf(stdout, L"[%s][%s]: %s\n", module, severity, text);
	}

	SetConsoleTextAttribute(h_stdout, _defaultColor);
}

void
Sys_TermDbgOut(void)
{

}

uint64_t
Sys_Time(void)
{
	LARGE_INTEGER freq, ctr;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ctr);

	return ((ctr.QuadPart / freq.QuadPart) * 1000000000UL) + ((ctr.QuadPart % freq.QuadPart) * 1000000000UL / freq.QuadPart);
}

bool
Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size)
{
	DWORD fileAccess, fileShare, mapAccess, protect;
	int prot = 0, flags = 0;

	if (!write) {
		mapAccess = FILE_MAP_READ;
		fileAccess = GENERIC_READ;
		fileShare = FILE_SHARE_READ;
		protect = PAGE_READONLY;
	} else {
		mapAccess = FILE_MAP_WRITE;
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileShare = FILE_SHARE_WRITE;
		protect = PAGE_READWRITE;
	}

	HANDLE file = CreateFileA(path, fileAccess, fileShare, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER liSize;
	GetFileSizeEx(file, &liSize);

	*size = liSize.QuadPart;

	HANDLE map = CreateFileMappingA(file, NULL, protect, 0, 0, NULL);
	if (map == INVALID_HANDLE_VALUE) {
		CloseHandle(file);
		return false;
	}

	*ptr = MapViewOfFile(map, mapAccess, 0, 0, 0);

	CloseHandle(file);
	CloseHandle(map);

	return true;
}

void
Sys_UnmapFile(const void *ptr, uint64_t size)
{
	UnmapViewOfFile(ptr);
}

uint32_t
Sys_TlsAlloc(void)
{
	return TlsAlloc();
}

void *
Sys_TlsGet(uint32_t key)
{
	return TlsGetValue(key);
}

void
Sys_TlsSet(uint32_t key, void *data)
{
	TlsSetValue(key, data);
}

void
Sys_TlsFree(uint32_t key)
{
	TlsFree(key);
}

void
Sys_Yield(void)
{
	SwitchToThread();
}

int
Sys_NumCpus(void)
{
	return _numCpus;
}

enum MachineType
Sys_MachineType(void)
{
	return MT_PC;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_ScreenVisible(void)
{
	return !IsIconic(E_Screen);
}

bool
Sys_UniversalWindows(void)
{
	return false;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
{
	UINT type = MB_OK;

	switch (icon) {
		case MSG_ICON_NONE:
			type = 0;
		break;
		case MSG_ICON_INFO:
			type |= MB_ICONINFORMATION;
		break;
		case MSG_ICON_WARN:
			type |= MB_ICONWARNING;
		break;
		case MSG_ICON_ERROR:
			type |= MB_ICONERROR;
		break;
	}

	MessageBox((HWND)E_Screen, message, title, type);
}

bool
Sys_ProcessEvents(void)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_INPUT) {
			HandleInput(msg.hwnd, msg.lParam, msg.wParam);
		} else if (msg.message == WM_QUIT) {
			return false;
		} else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}

bool
Sys_InitPlatform(void)
{
	SYSTEM_INFO si = { 0 };

	GetNativeSystemInfo(&si);

	_numCpus = si.dwNumberOfProcessors;

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	return true;
}

void
Sys_TermPlatform(void)
{
	CoUninitialize();
}

