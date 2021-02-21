#ifndef _SYS_MEMORY_H_
#define _SYS_MEMORY_H_

#include <stddef.h>
#include <stdbool.h>

enum MemoryHeap
{
	MH_Transient = 0,
	MH_Persistent,
	MH_Secure
};

void *Sys_Alloc(size_t size, size_t count, enum MemoryHeap heap);
void Sys_Free(void *mem);

void *Sys_AlignedAlloc(size_t size, size_t alignment);
void Sys_AlignedFree(void *mem);
void Sys_ZeroMemory(void *mem, size_t size);

bool Sys_InitMemory(void);
void Sys_ResetHeap(enum MemoryHeap heap);
void Sys_TermMemory(void);

#endif /* _SYS_MEMORY_H_ */
