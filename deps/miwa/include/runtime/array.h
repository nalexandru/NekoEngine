/* Miwa Portable Runtime
 *
 * array.h
 * Author: Alexandru Naiman
 *
 * Array
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019, Alexandru Naiman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MIWA_RUNTIME_ARRAY_H_
#define _MIWA_RUNTIME_ARRAY_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct rt_array - basic array
 * @data:
 * @count:
 * @size:
 * @elem_size
 */
typedef struct rt_array
{
	uint8_t *data;
	size_t count;
	size_t size;
	size_t elem_size;
} rt_array;

/**
 * rt_array_init - initialize a rt_array structure
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 * @elem_size: size of one array element
 */
int rt_array_init(rt_array *a, size_t size, size_t elem_size);

/**
 * rt_array_init_ptr - initialize a rt_array structure for pointers
 * @a: the structure to initialize
 * @size: initial array size (in elements)
 */
int rt_array_init_ptr(rt_array *a, size_t size);

/**
 * rt_array_clone - clone an array
 * @dst: destination array
 * @src: the array to clone
 *
 * Return: 0 on success
 */
int rt_array_clone(rt_array *dst, const rt_array *src);

/**
 * rt_array_first - get the first element of an array
 * @a: the array
 *
 * Returns: first element of the array
 */
//void *rt_array_first(rt_array *a);
#define rt_array_first(a) (a)->data

/**
 * rt_array_last - get the last element of an array
 * @a: the array
 *
 * Returns: last element of the array
 */
//void *rt_array_last(rt_array *a);
#define rt_array_last(a) ((a)->data + (a)->elem_size * ((a)->count - 1))

/**
 * rt_array_first - get the number of elements in an array
 * @a: the array
 *
 * Returns: number of elements
 */
//uint64_t rt_array_count(rt_array *a);
#define rt_array_count(a) (a)->count

/**
 * rt_array_size - get the size in elements of an array
 * @a: the array
 *
 * Returns: size in elements of the array
 */
//uint64_t rt_array_size(rt_array *a);
#define rt_array_size(a) (a)->size

/**
 * rt_array_size - get the size in bytes of an array
 * @a: the array
 *
 * Returns: size in bytes of the array
 */
//uint64_t rt_array_byte_size(rt_array *a);
#define rt_array_byte_size(a) ((a)->size * (a)->elem_size)

/**
 * rt_array_is_empty - check if an array has elements
 * @a: the array
 *
 * Returns: true if empty
 */
// bool rt_array_is_empty(rt_array *a)
#define rt_array_is_empty(a) (a)->count == 0

/**
 * rt_array_data_ptr - get the data pointer of an array
 * @a: the array
 *
 * Returns: data pointer
 */
//void *rt_array_data_ptr(rt_array *a);
#define rt_array_data_ptr(a) (a)->data

/**
 * rt_array_add - add an item to the array
 * @a: the array
 * @item: item to add
 *
 * &rt_array.elem_size number of bytes will be copied from item to the next
 * location in the array. If &rt_array.count is equal to &rt_array.size the
 * array will grow.
 *
 * Returns: OK on success
 */
int rt_array_add(rt_array *a, const void *item);

/**
 * rt_array_add - add a pointer to the array
 * @a: the array
 * @item: item to add
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_add()
 *
 * Returns: OK on success
 */
int rt_array_add_ptr(rt_array *a, const void *item);

/**
 * rt_array_create - allocate a new item in the array and return it
 * @a: the array
 *
 * Returns: Pointer to item
 */
void *rt_array_create(rt_array *a);

/**
 * rt_array_insert - insert an item in the array
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
int rt_array_insert(rt_array *a, const void *item, size_t pos);

/**
 * rt_array_insert_ptr - insert a pointer in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_insert()
 *
 * Returns: OK on success
 */
int rt_array_insert_ptr(rt_array *a, const void *item, size_t pos);

/**
 * rt_array_fast_insert - insert an item in the array
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
int rt_array_fast_insert(rt_array *a, const void *item, size_t pos);

/**
 * rt_array_fast_insert_ptr - insert a pointer in the array
 * @a: the array
 * @item: item to add
 * @pos: position to insert the item at
 *
 * Convenience function for arrays of pointers. Behaves exactly like
 * rt_array_fast_insert()
 *
 * Returns: OK on success
 */
int rt_array_fast_insert_ptr(rt_array *a, const void *item, size_t pos);

int rt_array_remove(rt_array *a, size_t index);
int rt_array_resize(rt_array *a, size_t size);

void *rt_array_get(const rt_array *a, size_t id);
void *rt_array_get_ptr(const rt_array *a, size_t id);

void *rt_array_find(const rt_array *a, const void *data, int32_t (*cmpfunc)(const void *item, const void *data));
size_t rt_array_find_id(const rt_array *a, const void *data, int32_t (*cmpfunc)(const void *item, const void *data));

void *rt_array_bsearch(const rt_array *a, const void *data, int32_t (*cmpfunc)(const void *item, const void *data));
size_t rt_array_bsearch_id(const rt_array *a, const void *data, int32_t (*cmpfunc)(const void *item, const void *data));

void rt_array_sort(rt_array *a, int32_t (*sortfunc)(const void *a, const void *b));
void rt_array_reverse(rt_array *a);

size_t rt_array_upper_bound(const rt_array *a, const void *data, int32_t (*cmpfunc)(const void *item, const void *data));
size_t rt_array_lower_bound(const rt_array *a, const void *data, int32_t (*cmpfunc)(const void *item, const void *data));

//void rt_array_fill(rt_array *a);
#define rt_array_fill(a) (a)->count = (a)->size
#define rt_array_zero(a) memset((a)->data, 0x0, (a)->elem_size * (a)->count)

void rt_array_clear(rt_array *a, bool free_memory);

void rt_array_release(rt_array *a);

#define rt_array_foreach(var, a)				\
	for (size_t miwa_rtafei = 0;				\
		(miwa_rtafei < (a)->count) &&			\
			(var = rt_array_get(a, miwa_rtafei));	\
		++miwa_rtafei)

#define rt_array_foreach_ptr(var, a)				\
	for (size_t miwa_rtafei = 0;				\
		(miwa_rtafei < (a)->count) &&			\
			(var = rt_array_get_ptr(a, miwa_rtafei));	\
		++miwa_rtafei)

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_RUNTIME_ARRAY_H_ */
