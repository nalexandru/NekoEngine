#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <System/AtomicLock.h>

#include <Render/Render.h>

#define MMOD L"MemoryManager"
#define MAGIC 0x53544954		// TITS, in little endian format

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

struct Allocation
{
	uint32_t magic;
	enum MemoryHeap heap;
	uint64_t size;
};

struct TransientHeap
{
	uint8_t *heap, *ptr;
	uint64_t *size;
	uint64_t peak;
	struct AtomicLock lock;
};

static inline bool _InitTransientHeap(struct TransientHeap *heap, uint64_t *size);
static inline void *_TransientAlloc(struct TransientHeap *heap, size_t size);
static inline void _ResetTransientHeap(struct TransientHeap *heap);

static struct TransientHeap _transientHeap;
static struct TransientHeap _frameHeap[RE_NUM_FRAMES];

void *
Sys_Alloc(size_t size, size_t count, enum MemoryHeap heap)
{
	void *ret = NULL;
	struct Allocation *alloc = NULL;
	size_t totalSize = size * count + sizeof(struct Allocation);

	if ((count >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && count > 0 && SIZE_MAX / count < size)
		return NULL;

	if (heap == MH_Transient) {
		totalSize = ROUND_UP(totalSize, 4);
		ret = _TransientAlloc(&_transientHeap, totalSize);
	} else if (heap == MH_Frame) {
		totalSize = ROUND_UP(totalSize, 4);
		ret = _TransientAlloc(&_frameHeap[Re_frameId], totalSize);
	} else {
		assert("Out of memory" && (ret = malloc(totalSize)));
//		assert("Out of memory" && (ret = Sys_AlignedAlloc(totalSize, 16))); FIXME
	}

	Sys_ZeroMemory(ret, totalSize);

	alloc = ret;
	alloc->magic = MAGIC;
	alloc->heap = heap;
	alloc->size = totalSize - sizeof(struct Allocation);

	ret = (uint8_t *)ret + sizeof(*alloc);

	return ret;
}

void *
Sys_ReAlloc(void *mem, size_t size, size_t count, enum MemoryHeap heap)
{
	void *new = NULL;
	struct Allocation *alloc;

	if (!mem)
		return Sys_Alloc(size, count, heap);

	size_t totalSize = size * count + sizeof(*alloc);
	if ((count >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && count > 0 && SIZE_MAX / count < size)
		return NULL;

	alloc = (struct Allocation *)((uint8_t *)mem - (sizeof(*alloc)));
	if (alloc->magic != MAGIC) {
		Sys_LogEntry(MMOD, LOG_DEBUG, L"Sys_ReAlloc called with unrecognized block %p, calling realloc.", mem);
		return realloc(mem, totalSize);
	}

	if (alloc->heap == MH_Transient || alloc->heap == MH_Frame) {
		assert(!"Attempt to realloc a transient block");
	} else {
		assert("Out of memory" && (new = realloc(alloc, totalSize)));
	}

	alloc = new;
	alloc->size = totalSize - sizeof(*alloc);

	return (uint8_t *)new + sizeof(*alloc);
}

void
Sys_Free(void *mem)
{
	struct Allocation *alloc;

	if (!mem)
		return;

	alloc = (struct Allocation *)((uint8_t *)mem - (sizeof(*alloc)));
	if (alloc->magic != MAGIC) {
		Sys_LogEntry(MMOD, LOG_DEBUG, L"Sys_Free called with unrecognized block %p, calling free.", mem);
		free(mem);
		return;
	}

	if (alloc->heap == MH_Transient || alloc->heap == MH_Frame)
		return;
	else if (alloc->heap == MH_Secure)
		Sys_ZeroMemory(alloc, alloc->size);

	free(alloc);
}

bool
Sys_InitMemory(void)
{
	if (!_InitTransientHeap(&_transientHeap, &E_GetCVarU64(L"Engine_TransientHeapSize", 6 * 1024 * 1024)->u64))
		return false;

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (!_InitTransientHeap(&_frameHeap[i], &E_GetCVarU64(L"Engine_FrameHeapSize", 2 * 1024 * 1024)->u64))
			return false;

	return true;
}

void
Sys_ResetHeap(enum MemoryHeap heap)
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
		Sys_LogEntry(MMOD, LOG_INFORMATION, L"Frame heap peak: %u/%u B (%.02f%%)", _frameHeap[i].peak, *_frameHeap[i].size,
			((double)_frameHeap[i].peak / (double)*_frameHeap[i].size) * 100.0);

	Sys_LogEntry(MMOD, LOG_INFORMATION, L"Transient heap peak: %u/%u B (%.02f%%)", _transientHeap.peak, *_transientHeap.size,
		((double)_transientHeap.peak / (double)*_transientHeap.size) * 100.0);
}

void
Sys_TermMemory(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		Sys_AlignedFree(_frameHeap[i].heap);

	Sys_AlignedFree(_transientHeap.heap);
}

static inline bool
_InitTransientHeap(struct TransientHeap *heap, uint64_t *size)
{
	heap->size = size;
	heap->heap = Sys_AlignedAlloc((size_t)*heap->size, 16);

	if (!heap->heap)
		return false;

	heap->ptr = heap->heap;

	return true;
}

static inline void *
_TransientAlloc(struct TransientHeap *heap, size_t size)
{
	Sys_AtomicLockWrite(&heap->lock);

	assert("Out of transient memory" && !((heap->ptr - heap->heap) + size > *heap->size));

	void *ret = heap->ptr;
	heap->ptr += size;

	Sys_AtomicUnlockWrite(&heap->lock);

	return ret;
}

static inline void
_ResetTransientHeap(struct TransientHeap *heap)
{
	Sys_AtomicLockWrite(&heap->lock);
	heap->peak = MAX(heap->peak, (size_t)(heap->ptr - heap->heap));
	heap->ptr = heap->heap;
	Sys_AtomicUnlockWrite(&heap->lock);
}
