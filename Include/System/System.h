#ifndef _SYS_SYSTEM_H_
#define _SYS_SYSTEM_H_

#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <Engine/Types.h>

enum MsgBoxIcon
{
	MSG_ICON_NONE = 0,
	MSG_ICON_INFO = 1,
	MSG_ICON_WARN = 2,
	MSG_ICON_ERROR = 3
};

enum MachineType
{
	MT_PC,
	MT_XBOX_ONE,
	MT_XBOX,
	MT_SWITCH,
	MT_WII,
	MT_MOBILE
};

enum SystemCapabilityFlags
{
	SC_MMIO = 0x01
};

enum SystemDirectory
{
	SD_SAVE_GAME,
	SD_APP_DATA,
	SD_TEMP
};

bool Sys_Init(void);
void Sys_Term(void);

bool Sys_InitDbgOut(void);
void Sys_DbgOut(int color, const wchar_t *module, const wchar_t *severity, const wchar_t *text);
void Sys_TermDbgOut(void);

uint64_t Sys_Time(void);

bool Sys_MapFile(const char *path, bool write, void **ptr, uint64_t *size);
void Sys_UnmapFile(const void *ptr, uint64_t size);

const char *Sys_Hostname(void);
const char *Sys_Machine(void);
const char *Sys_CpuName(void);
int32_t Sys_CpuFreq(void);
int32_t Sys_NumCpus(void);

const char *Sys_OperatingSystem(void);
const char *Sys_OperatingSystemVersion(void);

enum MachineType Sys_MachineType(void);
uint32_t Sys_Capabilities(void);

bool Sys_ScreenVisible(void);
void Sys_MessageBox(const wchar_t *title, const wchar_t *message, int icon);
bool Sys_ProcessEvents(void);

void *Sys_LoadLibrary(const char *path);
void *Sys_GetProcAddress(void *lib, const char *name);
void Sys_UnloadLibrary(void *lib);

void Sys_Sleep(uint32_t sec);
void Sys_MSleep(uint32_t msec);
void Sys_USleep(uint32_t usec);

void Sys_DirectoryPath(enum SystemDirectory sd, char *out, size_t len);
bool Sys_DirectoryExists(const char *path);
bool Sys_CreateDirectory(const char *path);

// Compatibility shivs
void *reallocarray(void *ptr, size_t nmemb, size_t size);

int getopt(int nargc, char *const nargv[], const char *ostr);

extern int opterr, optind, optopt, optreset;
extern char *optarg;

#endif /* _SYS_SYSTEM_H_ */
