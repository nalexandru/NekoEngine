#ifndef _SYS_MEMORY_H_
#define _SYS_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

enum MemoryHeap
{
	MH_Transient = 0,
	MH_Scene,
	MH_Persistent,
	MH_Secure
};

void *Sys_Alloc(size_t size, size_t count, enum MemoryHeap heap);
void Sys_Free(void *mem);

void *Sys_AlignedAlloc(size_t size, size_t alignment);
void Sys_AlignedFree(void *mem);

bool Sys_InitMemory(void);
void Sys_ResetHeap(enum MemoryHeap heap);
void Sys_TermMemory(void);

#ifdef __cplusplus
}
#endif

#endif