#include <stdint.h>
#include <assert.h>

#include <Windows.h>

#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Memory.h>

#define MMOD L"MemoryManager"
#define MAGIC 0xB15B00B5

#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

struct Allocation
{
	uint32_t magic;
	size_t size;
	enum MemoryHeap heap;
};

static uint8_t *_transientHeap, *_transientHeapPtr;
static uint8_t *_sceneHeap, *_sceneHeapPtr;
static uint64_t *_transientHeapSize;
static uint64_t _transientHeapPeak = 0;

void *
Sys_Alloc(size_t size, size_t count, enum MemoryHeap heap)
{
	void *ret = NULL;
	size_t totalSize = size * count;
	struct Allocation *alloc = NULL;

	// TODO: check overflow

	switch (heap) {
	case MH_Transient: {
		totalSize = ROUND_UP(totalSize, 4);
		if ((_transientHeapPtr - _transientHeap) + totalSize > *_transientHeapSize) {
			assert(!"Out of transient memory");
			break;
		}
		ret = _transientHeapPtr;
		_transientHeapPtr += totalSize;
	} break;
	case MH_Persistent:
	case MH_Secure: {
		ret = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalSize + sizeof(struct Allocation));
		if (!ret)
			return NULL;

		alloc = (struct Allocation *)ret;
		alloc->magic = MAGIC;
		alloc->heap = heap;
		alloc->size = totalSize;

		ret = (uint8_t *)ret + sizeof(*alloc);
	} break;
	}

	return ret;
}

void
Sys_Free(void *mem)
{
	struct Allocation *alloc;

	if (!mem)
		return;

	alloc = (struct Allocation *)((uint8_t *)mem - (sizeof(*alloc)));
	if (alloc->magic != MAGIC) {
		Sys_LogEntry(MMOD, LOG_CRITICAL, L"Attempt to free unrecognized block %p.", mem);
		return;
	}

	if (alloc->heap == MH_Secure) {
		SecureZeroMemory(alloc, alloc->size);
		alloc->heap = MH_Persistent;
	}

	switch (alloc->heap) {
	case MH_Persistent:
		HeapFree(GetProcessHeap(), 0, alloc);
	break;
	}

	// check end
	// free
}

void *
Sys_AlignedAlloc(size_t size, size_t alignment)
{
	return _aligned_malloc(size, alignment);
}

void
Sys_AlignedFree(void* mem)
{
	_aligned_free(mem);
}

bool
Sys_InitMemory(void)
{
	_transientHeapSize = &E_GetCVarU64(L"Engine_TransientHeapSize", 6 * 1024 * 1024)->u64;

	_transientHeap = (uint8_t *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *_transientHeapSize);
	if (!_transientHeap)
		return false;

	_transientHeapPtr = _transientHeap;

	return true;
}

void
Sys_ResetHeap(enum MemoryHeap heap)
{
	switch (heap) {
	case MH_Transient:
		_transientHeapPeak = max(_transientHeapPeak, (size_t)(_transientHeapPtr - _transientHeap));

		_transientHeapPtr = _transientHeap;
	break;
	}
}

void
Sys_TermMemory(void)
{
	Sys_LogEntry(MMOD, LOG_DEBUG, L"Transient heap peak: %u/%u B (%.02f%%)", _transientHeapPeak, *_transientHeapSize,
		((double)_transientHeapPeak / (double)*_transientHeapSize) * 100.0);

	HeapFree(GetProcessHeap(), 0, _transientHeap);
}
