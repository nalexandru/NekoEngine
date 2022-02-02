#ifndef _NE_SYSTEM_SYSTEM_H_
#define _NE_SYSTEM_SYSTEM_H_

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <Engine/Types.h>
#include <System/PlatformDetect.h>

enum NeMsgBoxIcon
{
	MSG_ICON_NONE = 0,
	MSG_ICON_INFO = 1,
	MSG_ICON_WARN = 2,
	MSG_ICON_ERROR = 3
};

enum NeMachineType
{
	MT_PC,
	MT_XBOX_ONE,
	MT_XBOX,
	MT_SWITCH,
	MT_WII,
	MT_MOBILE,
	MT_PS4
};

enum NeSystemCapabilityFlags
{
	SC_MMIO = 0x01
};

enum NeSystemDirectory
{
	SD_SAVE_GAME,
	SD_APP_DATA,
	SD_TEMP
};

struct NeSysVersion
{
	uint32_t major;
	uint32_t minor;
	uint32_t revision;
};

#if defined(__GNUC__) || defined(__clang__)
#	define likely(x)		__builtin_expect(!!(x), 1)
#	define unlikely(x)		__builtin_expect(!!(x), 0)
#else
#	define likely(x)		x
#	define unlikely(x)		x
#endif

bool Sys_Init(void);
void Sys_Term(void);

bool Sys_InitDbgOut(void);
void Sys_DbgOut(int color, const char *module, const char *severity, const char *text);
void Sys_TermDbgOut(void);

uint64_t Sys_Time(void);

bool Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size);
void Sys_UnmapFile(const void *ptr, uint64_t size);

const char *Sys_Hostname(void);
const char *Sys_Machine(void);
const char *Sys_CpuName(void);
uint32_t Sys_CpuFreq(void);
uint32_t Sys_CpuCount(void);
uint32_t Sys_CpuThreadCount(void);

const char *Sys_OperatingSystem(void);
const char *Sys_OperatingSystemVersionString(void);
struct NeSysVersion Sys_OperatingSystemVersion(void);;

enum NeMachineType Sys_MachineType(void);
uint32_t Sys_Capabilities(void);

bool Sys_ScreenVisible(void);
void Sys_MessageBox(const char *title, const char *message, int icon);
bool Sys_ProcessEvents(void);

void *Sys_LoadLibrary(const char *path);
void *Sys_GetProcAddress(void *lib, const char *name);
void Sys_UnloadLibrary(void *lib);

void Sys_Sleep(uint32_t sec);
void Sys_MSleep(uint32_t msec);
void Sys_USleep(uint32_t usec);

void Sys_DirectoryPath(enum NeSystemDirectory sd, char *out, size_t len);
bool Sys_FileExists(const char *path);
bool Sys_DirectoryExists(const char *path);
bool Sys_CreateDirectory(const char *path);

void Sys_ExecutableLocation(char *out, uint32_t len);

// Compatibility shivs
void *reallocarray(void *ptr, size_t nmemb, size_t size);

#if defined(SYS_PLATFORM_WINDOWS)

#include <direct.h>

#define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)

int getopt(int nargc, char *const nargv[], const char *ostr);

extern int opterr, optind, optopt, optreset;
extern char *optarg;

#elif defined(SYS_PLATFORM_UNIX) || defined(SYS_PLATFORM_NX)

#include <unistd.h>

#endif

#endif /* _NE_SYSTEM_SYSTEM_H_ */
