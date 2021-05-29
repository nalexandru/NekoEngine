#ifndef _NE_SYSTEM_MEMORY_H_
#define _NE_SYSTEM_MEMORY_H_

#include <stddef.h>
#include <stdbool.h>

enum MemoryHeap
{
	MH_Transient = 0,
	MH_Frame,

	MH_Secure,

	MH_Audio,
	MH_Render,

	MH_Scene,
	MH_Asset,
	MH_Script,

	MH_AudioDriver,
	MH_RenderDriver,

	MH_Debug,
	MH_System,

	MH_FORCE_UINT32 = 0xFFFFFFFF
};

void *Sys_Alloc(size_t size, size_t count, enum MemoryHeap heap);
void *Sys_ReAlloc(void *mem, size_t size, size_t count, enum MemoryHeap heap);
void Sys_Free(void *mem);

void *Sys_AlignedAlloc(size_t size, size_t alignment);
void Sys_AlignedFree(void *mem);
void Sys_ZeroMemory(void *mem, size_t size);

bool Sys_InitMemory(void);
void Sys_ResetHeap(enum MemoryHeap heap);
void Sys_LogMemoryStatistics(void);
void Sys_TermMemory(void);

#endif /* _NE_SYSTEM_MEMORY_H_ */

