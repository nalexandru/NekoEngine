#include <Windows.h>

#include <stdio.h>

#include <System/System.h>

static int _num_cpus = 0;
static HANDLE h_stdout;
static HANDLE h_stderr;

static WORD _colors[4] =
{
	13, 7, 14, 12
};
static WORD _defaultColor;

int
Sys_NumCpus(void)
{
	return _num_cpus;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_InitPlatform(void)
{
	SYSTEM_INFO si = { 0 };

	GetNativeSystemInfo(&si);

	_num_cpus = si.dwNumberOfProcessors;

	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
		return false;

	return true;
}

void
Sys_TermPlatform(void)
{
	CoUninitialize();
}

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
