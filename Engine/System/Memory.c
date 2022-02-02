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
#define DEFAULT_ALIGNMENT		16

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

struct NeAllocation
{
	uint32_t magic;
	enum NeMemoryHeap heap;
	uint64_t size;
};

struct NeTransientHeap
{
	uint8_t *heap, *ptr;
	uint64_t *size;
	uint64_t peak;
	struct NeAtomicLock lock;
};

static inline bool _InitTransientHeap(struct NeTransientHeap *heap, uint64_t *size);
static inline void *_TransientAlloc(struct NeTransientHeap *heap, size_t size);
static inline void _ResetTransientHeap(struct NeTransientHeap *heap);

static struct NeTransientHeap _transientHeap;
static struct NeTransientHeap _frameHeap[RE_NUM_FRAMES];

void *
Sys_Alloc(size_t size, size_t count, enum NeMemoryHeap heap)
{
	void *ret = NULL;
	struct NeAllocation *alloc = NULL;
	size_t totalSize = size * count + sizeof(struct NeAllocation);

	if ((count >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && count > 0 && SIZE_MAX / count < size)
		return NULL;

	totalSize = ROUND_UP(totalSize, DEFAULT_ALIGNMENT);
	if (heap == MH_Transient)
		ret = _TransientAlloc(&_transientHeap, totalSize);
	else if (heap == MH_Frame)
		ret = _TransientAlloc(&_frameHeap[Re_frameId], totalSize);
	else
		ret = aligned_alloc(DEFAULT_ALIGNMENT, totalSize);

	assert("Out of memory" && ret);
	if (!ret)
		return NULL;

	Sys_ZeroMemory(ret, totalSize);

	alloc = ret;
	alloc->magic = MAGIC;
	alloc->heap = heap;
	alloc->size = totalSize - sizeof(struct NeAllocation);

	ret = (uint8_t *)ret + sizeof(*alloc);

	return ret;
}

void *
Sys_ReAlloc(void *mem, size_t size, size_t count, enum NeMemoryHeap heap)
{
	void *new = NULL;
	struct NeAllocation *alloc;

	if (!mem)
		return Sys_Alloc(size, count, heap);

	size_t totalSize = size * count + sizeof(*alloc);
	if ((count >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && count > 0 && SIZE_MAX / count < size)
		return NULL;

	alloc = (struct NeAllocation *)((uint8_t *)mem - (sizeof(*alloc)));
	if (alloc->magic != MAGIC) {
		Sys_LogEntry(MMOD, LOG_DEBUG, "Sys_ReAlloc called with unrecognized block %p, calling realloc.", mem);
		return realloc(mem, totalSize);
	}

	if (alloc->heap == MH_Transient || alloc->heap == MH_Frame) {
		assert(!"Attempt to realloc a transient block");
	} else {
#ifndef SYS_PLATFORM_WINDOWS
		new = realloc(alloc, totalSize);
#else
		new = _aligned_realloc(alloc, totalSize, DEFAULT_ALIGNMENT);
#endif
		assert("Out of memory" && new);
	}

	if (!new)
		return NULL;

	alloc = new;
	alloc->size = totalSize - sizeof(*alloc);

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

	if (alloc->heap == MH_Transient || alloc->heap == MH_Frame)
		return;
	else if (alloc->heap == MH_Secure)
		Sys_ZeroMemory(alloc, alloc->size);

#ifndef SYS_PLATFORM_WINDOWS
	free(alloc);
#else
	_aligned_free(alloc);
#endif
}

bool
Sys_InitMemory(void)
{
	if (!_InitTransientHeap(&_transientHeap, &E_GetCVarU64("Engine_TransientHeapSize", 64 * 1024 * 1024)->u64))
		return false;

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (!_InitTransientHeap(&_frameHeap[i], &E_GetCVarU64("Engine_FrameHeapSize", 12 * 1024 * 1024)->u64))
			return false;

	return true;
}

void
Sys_ResetHeap(enum NeMemoryHeap heap)
{
	if (heap == MH_Transient)
		_ResetTransientHeap(&_transientHeap);
	else if (heap == MH_Frame)
		_ResetTransientHeap(&_frameHeap[Re_frameId]);
}

void
Sys_LogMemoryStatistics(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		Sys_LogEntry(MMOD, LOG_INFORMATION, "Frame heap peak: %u/%u B (%.02f%%)", _frameHeap[i].peak, *_frameHeap[i].size,
			((double)_frameHeap[i].peak / (double)*_frameHeap[i].size) * 100.0);

	Sys_LogEntry(MMOD, LOG_INFORMATION, "Transient heap peak: %u/%u B (%.02f%%)", _transientHeap.peak, *_transientHeap.size,
		((double)_transientHeap.peak / (double)*_transientHeap.size) * 100.0);
}

void
Sys_TermMemory(void)
{
#ifndef SYS_PLATFORM_WINDOWS
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		free(_frameHeap[i].heap);
	free(_transientHeap.heap);
#else
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		_aligned_free(_frameHeap[i].heap);
	_aligned_free(_transientHeap.heap);
#endif
}

static inline bool
_InitTransientHeap(struct NeTransientHeap *heap, uint64_t *size)
{
	heap->size = size;
	heap->heap = aligned_alloc(16, (size_t)*heap->size);

	if (!heap->heap)
		return false;

	heap->ptr = heap->heap;

	return true;
}

static inline void *
_TransientAlloc(struct NeTransientHeap *heap, size_t size)
{
	Sys_AtomicLockWrite(&heap->lock);

	assert("Out of transient memory" && !((heap->ptr - heap->heap) + size > *heap->size));

	void *ret = heap->ptr;
	heap->ptr += size;

	Sys_AtomicUnlockWrite(&heap->lock);

	return ret;
}

static inline void
_ResetTransientHeap(struct NeTransientHeap *heap)
{
	Sys_AtomicLockWrite(&heap->lock);
	heap->peak = MAX(heap->peak, (size_t)(heap->ptr - heap->heap));
	heap->ptr = heap->heap;
	Sys_AtomicUnlockWrite(&heap->lock);
}
