#include <stdio.h>
#include <Windows.h>

#if defined(_M_AMD64) || defined(_M_IX86)
#	include <intrin.h>
#endif

#include <System/System.h>
#include <Engine/Engine.h>

#include "Win32Platform.h"

static int _numCpus = 0;
static HANDLE _stdout;
static HANDLE _stderr;
static WORD _cpuArch, _colors[4] =
{
	13, 7, 14, 12
};
static WORD _defaultColor;
static char _hostname[MAX_COMPUTERNAME_LENGTH + 1], _cpu[50];

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

	_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	_stderr = GetStdHandle(STD_ERROR_HANDLE);

	GetConsoleScreenBufferInfo(_stdout, &csbi);
	_defaultColor = csbi.wAttributes;

	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	SetConsoleTextAttribute(_stdout, _colors[color]);

	if (IsDebuggerPresent()) {
		wchar_t buff[4096];
		swprintf(buff, 4096, L"[%ls][%ls]: %ls\n", module, severity, text);
		OutputDebugString(buff);
	} else {
		fwprintf(stdout, L"[%ls][%ls]: %ls\n", module, severity, text);
	}

	SetConsoleTextAttribute(_stdout, _defaultColor);
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

const char *
Sys_Hostname(void)
{
	DWORD size = sizeof(_hostname);

	if (!_hostname[0])
		GetComputerNameA(_hostname, &size);

	return _hostname;
}

const char *
Sys_Machine(void)
{
	switch (_cpuArch)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:			return "x64";
	case PROCESSOR_ARCHITECTURE_INTEL:			return "x86";
	case PROCESSOR_ARCHITECTURE_ARM64:			return "arm64";
	case PROCESSOR_ARCHITECTURE_ARM:			return "arm";
	case PROCESSOR_ARCHITECTURE_MIPS:			return "MIPS";
	case PROCESSOR_ARCHITECTURE_ALPHA:			return "Alpha";
	case PROCESSOR_ARCHITECTURE_ALPHA64:		return "Alpha 64";
	case PROCESSOR_ARCHITECTURE_SHX:			return "SuperH";
	case PROCESSOR_ARCHITECTURE_PPC:			return "PowerPC";
	case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:	return "Itanium on x64";
	case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64:	return "arm on x64";
	case PROCESSOR_ARCHITECTURE_IA64:			return "Itanium";
	case PROCESSOR_ARCHITECTURE_MSIL:			return "msil";
	default:									return "Unknown";
	}
}

const char *
Sys_CpuName(void)
{
	if (!_cpu[0]) {
#if defined(_M_AMD64) || defined(_M_IX86)
		int cpuInfo[4] = { 0 };

		__cpuid(cpuInfo, 0x80000002);
		memcpy(_cpu, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000003);
		memcpy(_cpu + 16, cpuInfo, sizeof(cpuInfo));

		__cpuid(cpuInfo, 0x80000004);
		memcpy(_cpu + 32, cpuInfo, sizeof(cpuInfo));
#elif defined(_M_ARM64)
#else // Unknown architecture, attempt to read from registry
		HKEY key;
		DWORD size = 0;

		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &key)
				== ERROR_SUCCESS) {
			size = sizeof(_cpu);
			RegQueryValueExA(key, "ProcessorNameString", 0, 0, (LPBYTE)_cpu, &size);

			RegCloseKey(key);
		}
#endif

		if (!_cpu[0])
			memcpy(_cpu, "Unknown", 7);
	}

	return _cpu;
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
	_cpuArch = si.wProcessorArchitecture;

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	return true;
}

void
Sys_TermPlatform(void)
{
	CoUninitialize();
}

