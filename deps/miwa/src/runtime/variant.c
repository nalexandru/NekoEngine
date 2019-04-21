/* Miwa Portable Runtime
 *
 * variant.c
 * Author: Alexandru Naiman
 *
 * Variant
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

#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>

#include <system/log.h>
#include <system/compat.h>
#include <runtime/runtime.h>

typedef struct vt_obj_field
{
	rt_string str;
	rt_variant val;
} vt_obj_field;

static INLINE vt_obj_field *
_vt_obj_get_field(rt_variant *vt,
	const char *name)
{
	size_t i = 0;
	uint64_t hash = 0;
	rt_array *a = 0;
	vt_obj_field *field = 0;

	if (!vt || !vt->data.ptr || !name)
		return NULL;

	a = vt->data.ptr;
	hash = rt_hash_string(name);

	for (i = 0; i < a->count; ++i) {
		field = rt_array_get(a, i);

		if (field->str.hash == hash)
			return field;
	}

	return NULL;
}

rt_variant *
rt_vt_string(rt_variant *vt,
	const char *s)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_STRING;
	vt->data.str.data = strdup(s);
	vt->data.str.len = strlen(s);

	return vt;
}

rt_variant *
rt_vt_int(rt_variant *vt,
	int64_t i)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_INT;
	vt->data.i = i;

	return vt;
}

rt_variant *
rt_vt_double(rt_variant * vt,
	double d)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_DOUBLE;
	vt->data.d = d;

	return vt;
}

rt_variant *
rt_vt_pointer(rt_variant *vt, void *ptr)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_POINTER;
	vt->data.ptr = ptr;

	return vt;
}

rt_variant *
rt_vt_bool(rt_variant * vt, bool b)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_BOOL;
	vt->data.b = b;

	return vt;
}

rt_variant *
rt_vt_array(rt_variant *vt)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_ARRAY;
	vt->data.ptr = calloc(1, sizeof(rt_array));

	if (!vt->data.ptr)
		return NULL;

	if (rt_array_init(vt->data.ptr, 10, sizeof(rt_variant)) != SYS_OK) {
		free(vt->data.ptr);
		return NULL;
	}

	return vt;
}

bool
rt_vt_array_append(rt_variant *vt,
	rt_variant *val)
{
	rt_variant clone;

	if (!vt || vt->type != RT_VT_ARRAY || !val)
		return false;

	rt_vt_copy(&clone, val);

	return rt_array_add(vt->data.ptr, &clone) == SYS_OK ? true : false;
}

rt_variant *
rt_vt_array_get(rt_variant *vt,
	size_t id)
{
	if (!vt || vt->type != RT_VT_ARRAY)
		return 0;

	return rt_array_get(vt->data.ptr, id);
}

size_t
rt_vt_array_count(rt_variant * vt)
{
	if (!vt || vt->type != RT_VT_ARRAY)
		return 0;

	return ((rt_array *)vt->data.ptr)->count;
}

rt_variant *
rt_vt_object(rt_variant *vt)
{
	if (!vt)
		return NULL;

	rt_vt_release(vt);

	vt->type = RT_VT_OBJECT;
	vt->data.ptr = calloc(1, sizeof(rt_array));

	if (!vt->data.ptr)
		return NULL;

	if (rt_array_init(vt->data.ptr, 10, sizeof(vt_obj_field)) != SYS_OK) {
		free(vt->data.ptr);
		return NULL;
	}

	return vt;
}

bool
rt_vt_object_set(rt_variant *vt,
	const char *name,
	rt_variant *val)
{
	vt_obj_field *field = 0, f;
	rt_variant tmp;
	bool ret = false;

	if (!vt || vt->type != RT_VT_OBJECT || !name || !val)
		return false;

	field = _vt_obj_get_field(vt, name);

	memset(&tmp, 0x0, sizeof(tmp));

	if (field) {
		rt_vt_release(&field->val);

		rt_vt_copy(&tmp, val);

		memcpy(&field->val, &tmp, sizeof(field->val));

		ret = true;
	} else {
		memset(&f, 0x0, sizeof(f));

		rt_vt_copy(&tmp, val);

		rt_string_init_with_cstr(&f.str, name);
		memcpy(&f.val, &tmp, sizeof(f.val));

		ret = rt_array_add(vt->data.ptr, &f);
	}

	return ret;
}

rt_variant *
rt_vt_object_get(rt_variant *vt,
	const char *name)
{
	vt_obj_field *field = 0;

	if (!vt || vt->type != RT_VT_OBJECT || !name)
		return NULL;

	field = _vt_obj_get_field(vt, name);

	if (!field)
		return NULL;

	return &field->val;
}

bool
rt_vt_object_get_id(const rt_variant *vt,
	size_t id,
	const char **key,
	rt_variant * *value)
{
	vt_obj_field *field = 0;

	if (!vt || vt->type != RT_VT_OBJECT)
		return false;

	field = rt_array_get(vt->data.ptr, id);
	if (!field)
		return false;

	if (key)
		* key = field->str.data;

	if (value)
		* value = &field->val;

	return true;
}

bool
rt_vt_object_remove(rt_variant * vt,
	const char *name)
{
	size_t i = 0;
	uint64_t hash = 0;
	rt_array *a = 0;
	vt_obj_field *field = 0;

	if (!vt || !vt->data.ptr || !name)
		return false;

	a = vt->data.ptr;
	hash = rt_hash_string(name);

	for (i = 0; i < a->count; ++i) {
		field = rt_array_get(a, i);
		if (field->str.hash == hash) {
			rt_vt_release(&field->val);
			break;
		}
	}

	if (i == a->count)
		return false;

	return rt_array_remove(a, i);
}

size_t
rt_vt_object_field_count(const rt_variant * vt)
{
	if (!vt || vt->type != RT_VT_OBJECT)
		return 0;

	return ((rt_array *)vt->data.ptr)->count;
}

bool
rt_vt_object_has_field(rt_variant * vt,
	const char *name)
{
	if (!vt || vt->type != RT_VT_OBJECT)
		return false;

	return _vt_obj_get_field(vt, name) != NULL;
}

rt_variant *
rt_vt_clone(rt_variant *src)
{
	rt_variant *dst = 0;

	if (!src)
		return 0;

	dst = calloc(1, sizeof(*dst));
	rt_vt_copy(dst, src);

	return dst;
}

void
rt_vt_copy(rt_variant * dst,
	const rt_variant * src)
{
	size_t i = 0;
	rt_array *a = 0;
	rt_variant tmp_vt;
	vt_obj_field *tmp_field = 0, field;

	if (!dst || !src)
		return;

	memset(&tmp_vt, 0x0, sizeof(tmp_vt));
	memset(dst, 0x0, sizeof(*dst));

	dst->type = src->type;

	switch (src->type) {
	case RT_VT_INT:
	{
		rt_vt_int(dst, src->data.i);
	} break;
	case RT_VT_DOUBLE:
	{
		rt_vt_double(dst, src->data.d);
	} break;
	case RT_VT_BOOL:
	{
		rt_vt_bool(dst, src->data.b);
	} break;
	case RT_VT_POINTER:
	{
		dst->data.ptr = src->data.ptr;
	} break;
	case RT_VT_STRING:
	{
		dst->data.str.len = src->data.str.len;
		dst->data.str.data = strdup(src->data.str.data);
	} break;
	case RT_VT_OBJECT:
	{
		a = src->data.ptr;

		dst->data.ptr = calloc(1, sizeof(rt_array));
		rt_array_init(dst->data.ptr, a->size, a->elem_size);

		for (i = 0; i < a->count; ++i) {
			tmp_field = rt_array_get(a, i);
			memset(&field, 0x0, sizeof(field));

			rt_string_init_with_cstr(&field.str, tmp_field->str.data);
			rt_vt_copy(&field.val, &tmp_field->val);

			rt_array_add(dst->data.ptr, &field);
		}
	} break;
	case RT_VT_ARRAY:
	{
		a = src->data.ptr;

		dst->data.ptr = calloc(1, sizeof(rt_array));
		rt_array_init(dst->data.ptr, a->size, a->elem_size);

		for (i = 0; i < a->count; ++i) {

			memset(&field, 0x0, sizeof(field));
			rt_vt_copy(&field.val, rt_array_get(a, i));

			rt_array_add(dst->data.ptr, &field.val);
		}

	} break;
	}
}

void
rt_vt_release(rt_variant * vt)
{
	rt_array *a = 0;
	size_t i = 0;
	vt_obj_field *field = 0;

	if (!vt)
		return;

	if (vt->type == RT_VT_STRING) {
		free(vt->data.str.data);
	} else if (vt->type == RT_VT_ARRAY) {
		a = vt->data.ptr;

		for (i = 0; i < a->count; ++i)
			rt_vt_release(rt_array_get(a, i));

		rt_array_release(a);
	} else if (vt->type == RT_VT_OBJECT) {
		a = vt->data.ptr;

		for (i = 0; i < a->count; ++i) {
			field = rt_array_get(a, i);
			rt_string_release(&field->str);
			rt_vt_release(&field->val);
		}

		rt_array_release(a);
	}
}

