#include <wrl.h>
#include <wrl/client.h>

#include <intrin.h>

#include <System/System.h>
#include <System/Memory.h>

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::Foundation;

extern bool _UWPWindowVisible;
extern bool _UWPWindowClosed;

static int _numCpus = 0;
static int _xboxOne = -1;
static char _hostname[MAX_COMPUTERNAME_LENGTH + 1], _cpu[50];
static WORD _cpuArch = 0;

bool
Sys_InitDbgOut(void)
{
	return true;
}

void
Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text)
{
	if (IsDebuggerPresent()) {
		wchar_t buff[4096];
		swprintf(buff, 4096, L"[%ls][%ls]: %ls\n", module, severity, text);
		OutputDebugString(buff);
	} else {
		fwprintf(stdout, L"[%ls][%ls]: %ls\n", module, severity, text);
	}
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

	wchar_t *buff = (wchar_t *)Sys_Alloc(sizeof(wchar_t), strlen(path) + 1, MH_Transient);
	mbstowcs(buff, path, strlen(path) + 1);

	CREATEFILE2_EXTENDED_PARAMETERS cf2ep =
	{
		sizeof(cf2ep),
		FILE_ATTRIBUTE_NORMAL,
		FILE_FLAG_SEQUENTIAL_SCAN
	};
	HANDLE file = CreateFile2(buff, fileAccess, fileShare, OPEN_EXISTING, &cf2ep);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	LARGE_INTEGER liSize;
	GetFileSizeEx(file, &liSize);

	*size = liSize.QuadPart;

	HANDLE map = CreateFileMappingW(file, NULL, protect, 0, 0, NULL);
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
	if (_xboxOne < 0) {
		int cpu_info[4] = { 0 };
		char cpu_name[48] = { 0 };

		__cpuid(cpu_info, 0x80000002);
		memcpy(cpu_name, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000003);
		memcpy(cpu_name + 16, cpu_info, sizeof(cpu_info));

		__cpuid(cpu_info, 0x80000004);
		memcpy(cpu_name + 32, cpu_info, sizeof(cpu_info));

		_xboxOne = strstr(cpu_name, "Microsoft Xbox One") != NULL;
	}

	return _xboxOne ? MT_XBOX_ONE : MT_PC;
}

uint32_t
Sys_Capabilities(void)
{
	return SC_MMIO;
}

bool
Sys_ScreenVisible(void)
{
	return _UWPWindowVisible;
}

bool
Sys_UniversalWindows(void)
{
	return true;
}

void
Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon)
{
	MessageDialog ^md = ref new MessageDialog(ref new String(message), ref new String(title));
	IAsyncOperation<IUICommand ^> ^op = md->ShowAsync();
	
	while (op->Status == AsyncStatus::Started)
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}

bool
Sys_ProcessEvents(void)
{
	if (_UWPWindowClosed)
		return false;

	CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

	return true;
}

void *
Sys_LoadLibrary(const char *path)
{
	wchar_t *wpath = NULL;

	if (path) {
		wpath = (wchar_t *)Sys_Alloc(sizeof(wchar_t), strlen(path) + 1, MH_Transient);
		mbstowcs(wpath, path, strlen(path));
	}

	return LoadPackagedLibrary(wpath, 0);
}

void *
Sys_GetProcAddress(void *lib, const char *name)
{
	return GetProcAddress((HMODULE)lib, name);
}

void
Sys_UnloadLibrary(void *lib)
{
	FreeLibrary((HMODULE)lib);
}

extern "C" bool
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

extern "C" void
Sys_TermPlatform(void)
{
	CoUninitialize();
}

void *
Sys_AlignedAlloc(size_t size, size_t alignment)
{
	return _aligned_malloc(size, alignment);
}

void
Sys_AlignedFree(void *mem)
{
	_aligned_free(mem);
}

void
Sys_ZeroMemory(void *mem, size_t size)
{
	SecureZeroMemory(mem, size);
}

