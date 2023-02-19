#ifndef _NE_SYSTEM_MEMORY_H_
#define _NE_SYSTEM_MEMORY_H_

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum NeMemoryHeap
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

	MH_Editor,

	MH_ManualAlign,

	MH_FORCE_UINT32 = 0xFFFFFFFF
};

void *Sys_Alloc(size_t size, size_t count, enum NeMemoryHeap heap);
void *Sys_ReAlloc(void *mem, size_t size, size_t count, enum NeMemoryHeap heap);
void Sys_Free(void *mem);

void Sys_ZeroMemory(void *mem, size_t size);

bool Sys_InitMemory(void);
void Sys_ResetHeap(enum NeMemoryHeap heap);
void Sys_LogMemoryStatistics(void);
void Sys_TermMemory(void);

#ifdef __cplusplus
}
#endif

#endif /* _NE_SYSTEM_MEMORY_H_ */

/* NekoEngine
 *
 * Memory.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
