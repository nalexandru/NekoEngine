#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <System/AtomicLock.h>

#include <Render/Render.h>

#define MMOD					"MemoryManager"
#define MAGIC					0x53544954		// TITS, in little endian format

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct NeAllocation
{
	uint32_t magic;
	enum NeMemoryHeap heap;
	uint64_t size;
};

struct NeHeap
{
	uint8_t *heap, *ptr;
	uint64_t *size;
	uint64_t peak;
};

static inline bool InitHeap(struct NeHeap *heap, uint64_t *size);
static inline void *HeapAllocate(struct NeHeap *heap, size_t size, size_t alignment);
static inline void ResetHeap(struct NeHeap *heap);
static inline void TermHeap(struct NeHeap *heap);
static void ResetTransientJob(int workerId, void *data);
static void ResetFrameJob(int workerId, void *data);

static THREAD_LOCAL struct NeHeap f_transientHeap;
static THREAD_LOCAL struct NeHeap f_frameHeap[RE_NUM_FRAMES];

void *
Sys_AlignedAlloc(size_t size, size_t count, size_t alignment, enum NeMemoryHeap heap)
{
	void *ret = NULL;
	struct NeAllocation *alloc = NULL;

	size_t totalSize = size * count + sizeof(struct NeAllocation);
	if ((count >= NE_MUL_NO_OVERFLOW || size >= NE_MUL_NO_OVERFLOW) && count > 0 && SIZE_MAX / count < size)
		return NULL;

	totalSize = NE_ROUND_UP(totalSize, alignment);
	if (heap == MH_Transient)
		ret = HeapAllocate(&f_transientHeap, totalSize, alignment);
	else if (heap == MH_Frame)
		ret = HeapAllocate(&f_frameHeap[Re_frameId], totalSize, alignment);
	else
		ret = aligned_alloc(alignment, totalSize);

	assert("Out of memory" && ret);
	if (!ret)
		return NULL;

	Sys_ZeroMemory(ret, totalSize);

	alloc = ret;
	alloc->magic = MAGIC;
	alloc->heap = heap;
	alloc->size = totalSize - sizeof(struct NeAllocation);

	if (alloc->heap == MH_Secure)
		Sys_LockMemory(alloc, alloc->size);

	ret = (uint8_t *)ret + sizeof(*alloc);

	return ret;
}

void *
Sys_AlignedReAlloc(void *mem, size_t size, size_t count, size_t alignment, enum NeMemoryHeap heap)
{
	void *new = NULL;
	struct NeAllocation *alloc;

	if (!mem)
		return Sys_Alloc(size, count, heap);

	size_t totalSize = size * count + sizeof(*alloc);
	if ((count >= NE_MUL_NO_OVERFLOW || size >= NE_MUL_NO_OVERFLOW) && count > 0 && SIZE_MAX / count < size)
		return NULL;

	alloc = (struct NeAllocation *)((uint8_t *)mem - (sizeof(*alloc)));
	if (alloc->magic != MAGIC) {
		Sys_LogEntry(MMOD, LOG_DEBUG, "Sys_ReAlloc called with unrecognized block %p, calling realloc.", mem);
		return realloc(mem, totalSize);
	}

	if (heap == MH_Transient) {
		new = HeapAllocate(&f_transientHeap, totalSize, alignment);
		memcpy(new, mem, alloc->size);
	} else if (heap == MH_Frame) {
		new = HeapAllocate(&f_frameHeap[Re_frameId], totalSize, alignment);
		memcpy(new, mem, alloc->size);
	} else {
#ifndef SYS_PLATFORM_WINDOWS
		new = realloc(alloc, totalSize);
#else
		new = _aligned_realloc(alloc, totalSize, alignment);
#endif
		assert("Out of memory" && new);
	}

	if (!new)
		return NULL;

	alloc = new;
	alloc->size = totalSize - sizeof(*alloc);

	if (alloc->heap == MH_Secure)
		Sys_LockMemory(alloc, alloc->size);

	return (uint8_t *)new + sizeof(*alloc);
}

void
Sys_Free(void *mem)
{
	struct NeAllocation *alloc;

	if (!mem)
		return;

	alloc = (struct NeAllocation *)((uint8_t *)mem - (sizeof(*alloc)));
	if (alloc->magic != MAGIC) {
		Sys_LogEntry(MMOD, LOG_DEBUG, "Sys_Free called with unrecognized block %p, calling free.", mem);
		free(mem);
		return;
	}

	if (alloc->heap == MH_Transient || alloc->heap == MH_Frame) {
		return;
	} else if (alloc->heap == MH_Secure) {
		Sys_ZeroMemory(alloc, alloc->size);
		Sys_UnlockMemory(alloc, alloc->size);
	}

#ifndef SYS_PLATFORM_WINDOWS
	free(alloc);
#else
	_aligned_free(alloc);
#endif
}

bool
Sys_InitMemory(void)
{
	if (!InitHeap(&f_transientHeap, &E_GetCVarU64("Engine_TransientHeapSize", 4 * 1024 * 1024)->u64))
		return false;

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (!InitHeap(&f_frameHeap[i], &E_GetCVarU64("Engine_FrameHeapSize", 4 * 1024 * 1024)->u64))
			return false;

	return true;
}

void
Sys_ResetHeap(enum NeMemoryHeap heap)
{
	if (heap == MH_Transient) {
		ResetHeap(&f_transientHeap);
		E_DispatchJobs(E_JobWorkerThreads(), ResetTransientJob, NULL, NULL, NULL);
	} else if (heap == MH_Frame) {
		ResetHeap(&f_frameHeap[Re_frameId]);
		E_DispatchJobs(E_JobWorkerThreads(), ResetFrameJob, NULL, NULL, NULL);
	}
}

void
Sys_LogMemoryStatistics(void)
{
	if (E_WorkerId() == E_JobWorkerThreads()) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
			Sys_LogEntry(MMOD, LOG_INFORMATION, "Main thread frame heap peak: %u/%u B (%.02f%%)",
						f_frameHeap[i].peak, *f_frameHeap[i].size,
						((double)f_frameHeap[i].peak / (double)*f_frameHeap[i].size) * 100.0);

		Sys_LogEntry(MMOD, LOG_INFORMATION, "Main thread transient heap peak: %u/%u B (%.02f%%)",
					f_transientHeap.peak, *f_transientHeap.size,
					((double)f_transientHeap.peak / (double)*f_transientHeap.size) * 100.0);
	} else {
		for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
			Sys_LogEntry(MMOD, LOG_INFORMATION, "Worker %d frame heap peak: %u/%u B (%.02f%%)",
						E_WorkerId(), f_frameHeap[i].peak, *f_frameHeap[i].size,
						((double)f_frameHeap[i].peak / (double)*f_frameHeap[i].size) * 100.0);

		Sys_LogEntry(MMOD, LOG_INFORMATION, "Worker %d transient heap peak: %u/%u B (%.02f%%)",
					E_WorkerId(), f_transientHeap.peak, *f_transientHeap.size,
					((double)f_transientHeap.peak / (double)*f_transientHeap.size) * 100.0);
	}
}

void
Sys_TermMemory(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		TermHeap(&f_frameHeap[i]);
	TermHeap(&f_transientHeap);
}

static inline bool
InitHeap(struct NeHeap *heap, uint64_t *size)
{
	heap->size = size;
	heap->heap = aligned_alloc(NE_DEFAULT_ALIGNMENT, (size_t)*heap->size);

	if (!heap->heap)
		return false;

	heap->ptr = heap->heap;

	return true;
}

static inline void *
HeapAllocate(struct NeHeap *heap, size_t size, size_t alignment)
{
	heap->ptr = (uint8_t *)NE_ROUND_UP((uintptr_t)heap->ptr, alignment);

	if ((heap->ptr - heap->heap) + size > *heap->size) {
		Sys_LogEntry(MMOD, LOG_CRITICAL, "Worker %d: Out of transient memory ! Alloc Size = %llu", E_WorkerId(), size);
		assert(!"Out of transient memory");
	}

	void *ret = heap->ptr;
	heap->ptr += size;

	return ret;
}

static inline void
ResetHeap(struct NeHeap *heap)
{
	heap->peak = MAX(heap->peak, (size_t)(heap->ptr - heap->heap));
	heap->ptr = heap->heap;
}

static inline void
TermHeap(struct NeHeap *heap)
{
#ifndef SYS_PLATFORM_WINDOWS
	free(heap->heap);
#else
	_aligned_free(heap->heap);
#endif
}

static void
ResetTransientJob(int workerId, void *data)
{
	ResetHeap(&f_transientHeap);
}

static void
ResetFrameJob(int workerId, void *data)
{
	ResetHeap(&f_frameHeap[Re_frameId]);
}

/* NekoEngine
 *
 * Memory.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
