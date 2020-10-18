#ifndef _RT_ARRAY_H_
#define _RT_ARRAY_H_

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include <System/System.h>
#include <System/Memory.h>
#include <Runtime/RtDefs.h>

/**
 * struct rt_array - basic array
 * @data:
 * @count:
 * @size:
 * @elem_size
 */
typedef struct Array
{
	uint8_t *data;
	size_t count;
	size_t size;
	size_t elem_size;
	size_t align;
} Array;

/**
 * Rt_InitArray - initialize a rt_array structure
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @elem_size: size of one array element
 */
static inline bool
Rt_InitArray(Array *a, size_t size, size_t elem_size)
{
	if (!a || !size || !elem_size)
		return false;

	memset(a, 0x0, sizeof(*a));

	// seriously, C++ is retarded for needing this cast
	a->data = (uint8_t *)calloc(size, elem_size);

	if (!a->data)
		return false;

	a->count = 0;
	a->size = size;
	a->elem_size = elem_size;
	a->align = 1;

	return true;
}

/**
 * rt_array_init_align - initialize a rt_array structure with
 * data aligned to the specified boundary. The array will
 * have padding between items if necessary.
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @elem_size: size of one array element
 */
static inline bool
Rt_InitAlignedArray(Array *a, size_t size, size_t elem_size, size_t alignment)
{
	if (!a || !size || !elem_size)
		return false;

	if (alignment == 1)
		return Rt_InitArray(a, size, elem_size);

	memset(a, 0x0, sizeof(*a));

	// seriously, C++ is retarded for needing this cast
	a->data = (uint8_t *)Sys_AlignedAlloc(size * elem_size, alignment);

	if (!a->data)
		return false;

	a->count = 0;
	a->size = size;
	a->elem_size = (elem_size + alignment - 1) & ~(alignment - 1);
	a->align = alignment;

	return true;
}

/**
 * Rt_InitPtrArray - initialize a rt_array structure for pointers
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 */
#define Rt_InitPtrArray(a, size) Rt_InitArray(a, size, sizeof(void *))

/**
 * Rt_CloneArray - clone an array
 * @dst: destination array
 * @src: the array to clone
 *
 * Return: 0 on success
 */
static inline bool
Rt_CloneArray(Array *dst, const Array *src)
{
	if (!dst || !src)
		return false;

	dst->data = (uint8_t *)calloc(src->size, src->elem_size);
	if (!dst->data)
		return false;

	memcpy(dst->data, src->data, src->size * src->elem_size);

	dst->size = src->size;
	dst->count = src->count;
	dst->elem_size = src->elem_size;

	return true;
}

/**
 * Rt_ArrayFirst - get the first element of an array
 * @a: the array
 *
 * Returns: first element of the array
 */
#define Rt_ArrayFirst(a) (a)->data

/**
 * Rt_ArrayLast - get the last element of an array
 * @a: the array
 *
 * Returns: last element of the array
 */
#define Rt_ArrayLast(a) ((a)->data + (a)->elem_size * ((a)->count - 1))

/**
 * Rt_ArrayCount - get the number of elements in an array
 * @a: the array
 *
 * Returns: number of elements
 */
#define Rt_ArrayCount(a) (a)->count

/**
 * Rt_ArraySize - get the size in elements of an array
 * @a: the array
 *
 * Returns: size in elements of the array
 */
#define Rt_ArraySize(a) (a)->size

/**
 * Rt_ArrayByteSize - get the size in bytes of an array
 * @a: the array
 *
 * Returns: size in bytes of the array
 */
#define Rt_ArrayByteSize(a) ((a)->size * (a)->elem_size)

/**
 * Rt_ArrayDataPtr - get the data pointer of an array
 * @a: the array
 *
 * Returns: data pointer
 */
//void *rt_array_data_ptr(rt_array *a);
#define Rt_ArrayDataPtr(a) (a)->data

static inline int
Rt_ResizeArray(Array *a, size_t size)
{
	uint8_t *ptr = a->data;

	if (a->size == size)
		return true;

	if ((a->data = (uint8_t *)reallocarray(a->data, size, a->elem_size)) == NULL) {
		a->data = ptr;
		return false;
	}

	a->size = size;

	if (a->size < a->count)
		a->count = a->size;

	return true;
}

/**
 * Rt_ArrayAdd - add an item to the array
 * @a: the array
 * @item: item to add
 *
 * &rt_array.elem_size number of bytes will be copied from item to the next
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayAdd(Array *a, const void *data)
{
	if (a->count == a->size)
		if (!Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size,
			a->elem_size, a->size + RT_DEF_INC)))
			return false;

	memcpy(a->data + a->elem_size * a->count++, data, a->elem_size);

	return true;
}

/**
 * Rt_ArrayAddPtr - add a pointer to the array
 * @a: the array
 * @item: item to add
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_add()
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayAddPtr(Array *a, const void *data)
{
	return Rt_ArrayAdd(a, &data);
}

/**
 * Rt_ArrayAllocate - allocate a new item in the array and return it
 * @a: the array
 *
 * Returns: Pointer to item
 */
static inline void *
Rt_ArrayAllocate(Array *a)
{
	void *ptr = NULL;

	if (a->count == a->size)
		if (!Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size,
				a->elem_size, a->size + RT_DEF_INC)))
			return NULL;

	ptr = a->data + a->elem_size * a->count++;
	memset(ptr, 0x0, a->elem_size);

	return ptr;
}

// Actual insert function
static inline bool __miwa_array_insert(Array *, const void *, size_t, bool);

/**
 * Rt_ArrayInsert - insert an item in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * &rt_array.elem_size number of bytes will be copied from item to the @pos
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayInsert(Array *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, item, pos, true);
}

/**
 * Rt_ArrayInsertPtr - insert a pointer in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_insert()
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayInsertPtr(Array *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, &item, pos, true);
}

/**
 * Rt_ArrayFastInsert - insert an item in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * &rt_array.elem_size number of bytes will be copied from item to the @pos
 * location in the array. The item at @pos will be moved to the end of the
 * array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayFastInsert(Array *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, item, pos, false);
}

/**
 * Rt_ArrayFastInsertPtr - insert a pointer in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_fast_insert()
 *
 * Returns: OK on success
 */
static inline bool
Rt_ArrayFastInsertPtr(Array *a, const void *item, size_t pos)
{
	return __miwa_array_insert(a, &item, pos, false);
}

static inline bool
Rt_ArrayRemove(Array *a, size_t index)
{
	size_t i = 0;

	if (index > a->count - 1)
		return false;

	--a->count;

	for (i = index + 1; i <= a->count; ++i)
		memcpy(a->data + a->elem_size * (i - 1), a->data + a->elem_size * i, a->elem_size);

	return true;
}

static inline void *
Rt_ArrayGet(const Array *a, size_t id)
{
	if (id > a->size)
		return NULL;

	return a->data + a->elem_size * id;
}

static inline void *
Rt_ArrayGetPtr(const Array *a, size_t id)
{
	if (id > a->size)
		return NULL;

	return *(void **)Rt_ArrayGet(a, id);
}

static inline void *
Rt_ArrayFind(const Array *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (!cmpFunc(a->data + a->elem_size * i, data))
			return a->data + a->elem_size * i;

	return NULL;
}

static inline size_t
Rt_ArrayFindId(const Array *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t i;

	for (i = 0; i < a->count; ++i)
		if (!cmpFunc(a->data + a->elem_size * i, data))
			return i;

	return RT_NOT_FOUND;
}

static inline void *
Rt_ArrayBSearch(const Array *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t start = 0, end = a->count - 1, mid;
	int32_t c;
	void *elem = 0;

	if (!a->count)
		return NULL;

	while (start <= end && start < a->count && end < a->count) {
		mid = start + (end - start) / 2;

		elem = a->data + a->elem_size * mid;

		c = cmpFunc(elem, data);
		if (!c)
			return elem;
		else if (c < 0)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return NULL;
}

static inline size_t
Rt_ArrayBSearchId(const Array *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t start = 0, end = a->count - 1, mid;
	int32_t c;
	void *elem = 0;

	if (!a->count)
		return RT_NOT_FOUND;

	while (start <= end && start < a->count && end < a->count) {
		mid = start + (end - start) / 2;

		elem = a->data + a->elem_size * mid;

		c = cmpFunc(elem, data);
		if (!c)
			return mid;
		else if (c < 0)
			start = mid + 1;
		else
			end = mid - 1;
	}

	return RT_NOT_FOUND;
}

static inline void
Rt_ArraySort(Array *a, RtSortFunc sortFunc)
{
	qsort(a->data, a->count, a->elem_size, sortFunc);
}

static inline bool
Rt_ArrayReverse(Array *a)
{
	uint8_t *tmp = NULL;
	uint64_t s = 0, e = 0;
	void *start = 0, *end = 0;

	if (!a->count)
		return false;

	tmp = (uint8_t *)calloc(1, a->elem_size);
	if (!tmp)
		return false;

	s = 0;
	e = a->count - 1;

	while (s < e) {
		start = a->data + a->elem_size * s++;
		end = a->data + a->elem_size * e--;

		memcpy(tmp, start, a->elem_size);
		memcpy(start, end, a->elem_size);
		memcpy(end, tmp, a->elem_size);
	}

	free(tmp);

	return true;
}

static inline size_t
Rt_ArrayUpperBound(const Array *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t low = 0, mid = 0;
	size_t high = a->count;

	while (low < high) {
		mid = (low + high) / 2;

		if (cmpFunc(a->data + a->elem_size * mid, data) < 0)
			high = mid;
		else
			low = mid + 1;
	}

	return low;
}

static inline size_t
Rt_ArrayLowerBound(const Array *a, const void *data, RtCmpFunc cmpFunc)
{
	size_t low = 0, mid = 0;
	size_t high = a->count;

	while (low < high) {
		mid = (low + high) / 2;

		if (cmpFunc(a->data + a->elem_size * mid, data) >= 0)
			high = mid;
		else
			low = mid + 1;
	}

	return low;
}

#define Rt_FillArray(a) (a)->count = (a)->size
#define Rt_ZeroArray(a) memset((a)->data, 0x0, (a)->elem_size * (a)->count); a->count = 0

static inline void
Rt_ClearArray(Array *a, bool free_memory)
{
	a->count = 0;

	if (!free_memory)
		return;

	a->size = 0;

	if (a->align > 1)
		Sys_AlignedFree(a->data);
	else
		free(a->data);

	a->data = NULL;
}

static inline void
Rt_TermArray(Array *a)
{
	if (!a)
		return;

	Rt_ClearArray(a, true);
}

#define Rt_ArrayForEach(var, a)				\
	for (size_t miwa_rtafei = 0;				\
		(miwa_rtafei < (a)->count) &&			\
			(var = Rt_ArrayGet(a, miwa_rtafei));	\
		++miwa_rtafei)

#define Rt_ArrayForEachPtr(var, a)				\
	for (size_t miwa_rtafei = 0;				\
		(miwa_rtafei < (a)->count) &&			\
			(var = Rt_ArrayGetPtr(a, miwa_rtafei));	\
		++miwa_rtafei)

// You are not supposed to call this function directly;
// Use the wrappers defined above instead.
static inline bool
__miwa_array_insert(Array *a, const void *data, size_t pos, bool ordered)
{
	size_t i = 0;

	if (a->count == a->size)
		if (Rt_ResizeArray(a, _Rt_CalcGrowSize(a->size,
				a->elem_size, a->size + RT_DEF_INC)))
			return false;

	if (!ordered) {
		memcpy(a->data + a->elem_size * a->count, Rt_ArrayGet(a, pos), a->elem_size);
	} else {
		for (i = a->count; i > pos; --i)
			memcpy(a->data + a->elem_size * i, a->data + a->elem_size * (i - 1), a->elem_size);
	}

	memcpy(a->data + a->elem_size * pos, data, a->elem_size);
	++a->count;

	return true;
}


#endif /* _MIWA_RUNTIME_ARRAY_H_ */

