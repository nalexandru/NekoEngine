#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <unistd.h>

#include <Engine/Config.h>
#include <System/Log.h>
#include <System/Memory.h>

#define MMOD L"MemoryManager"
#define MAGIC 0xB15B00B5

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

#if (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)
#define	USE_POSIX_MEMALIGN
#elif (defined(__GLIBC__) && ((__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 8) || __GLIBC__ > 2) && defined(__LP64__)) \
		|| (defined(__FreeBSD__) && !defined(__arm__) && !defined(__mips__)) \
		|| defined(__APPLE__)
#define	USE_MALLOC
#elif __STDC_VERSION__ >= 201112L
#define USE_ALIGNED_ALLOC
#elif __SSE__
#include <intrin.h>
#define USE_MM_MALLLOC
#else
#define USE_MEMALIGN
#endif

struct Allocation
{
	uint32_t magic;
	size_t size;
	enum MemoryHeap heap;
};

static uint8_t *_transientHeap, *_transientHeapPtr;
//static uint8_t *_sceneHeap, *_sceneHeapPtr;
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
		ret = calloc(1, totalSize + sizeof(struct Allocation));
		if (!ret)
			return NULL;

		alloc = ret;
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
		explicit_bzero(alloc, alloc->size);
		alloc->heap = MH_Persistent;
	}

	switch (alloc->heap) {
	case MH_Persistent:
		free(alloc);
	break;
	}

	// check end
	// free
}

void *
Sys_AlignedAlloc(size_t size, size_t alignment)
{
#if defined(USE_POSIX_MEMALIGN)
	void *mem;
	if (posix_memalign(&mem, alignment, size))
		return NULL;
	return mem;
#elif defined(USE_MEMALIGN)
	return memalign(alignment, size);
#elif defined(USE_MALLOC)
	return malloc(size);
#elif defined(USE_ALIGNED_ALLOC)
	return aligned_alloc(alignment, size);
#elif defined(USE_MM_MALLOC)
	return _mm_malloc(size, alignment);
#else
#error	Aligned memory allocation not implemented for this platform
#endif
}

void
Sys_AlignedFree(void *mem)
{
#if defined(USE_MALLOC) || defined(USE_ALIGNED_ALLOC) || defined(USE_MEMALIGN) || defined(USE_POSIX_MEMALIGN)
	return free(mem);
#elif defined(USE_MM_MALLOC)
	return _mm_free(mem);
#else
#error	Aligned memory allocation not implemented for this platform
#endif
}

bool
Sys_InitMemory(void)
{
	_transientHeapSize = &E_GetCVarU64(L"Engine_TransientHeapSize", 6 * 1024 * 1024)->u64;

	_transientHeap = calloc(1, *_transientHeapSize);
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
		_transientHeapPeak = MAX(_transientHeapPeak, (size_t)(_transientHeapPtr - _transientHeap));

		_transientHeapPtr = _transientHeap;
	break;
	}
}

void
Sys_TermMemory(void)
{
	Sys_LogEntry(MMOD, LOG_DEBUG, L"Transient heap peak: %u/%u B (%.02f%%)", _transientHeapPeak, *_transientHeapSize,
		((double)_transientHeapPeak / (double)*_transientHeapSize) * 100.0);

	free(_transientHeap);
}
