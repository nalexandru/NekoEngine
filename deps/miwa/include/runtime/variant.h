/* Miwa Portable Runtime
 *
 * variant.h
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

#ifndef _MIWA_RUNTIME_VARIANT_H_
#define _MIWA_RUNTIME_VARIANT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum rt_variant_type
{
	RT_VT_STRING,
	RT_VT_INT,
	RT_VT_DOUBLE,
	RT_VT_POINTER,
	RT_VT_BOOL,
	RT_VT_ARRAY,
	RT_VT_OBJECT
} rt_variant_type;

typedef struct rt_variant
{
	rt_variant_type type;
	union
	{
		struct
		{
			char *data;
			size_t len;
		} str;
		int64_t i;
		double d;
		void *ptr;
		bool b;
	} data;
} rt_variant;

rt_variant	*rt_vt_string(rt_variant *vt, const char *s);
rt_variant	*rt_vt_int(rt_variant *vt, int64_t i);
rt_variant	*rt_vt_double(rt_variant *vt, double d);
rt_variant	*rt_vt_pointer(rt_variant *vt, void *ptr);
rt_variant	*rt_vt_bool(rt_variant *vt, bool b);
rt_variant	*rt_vt_array(rt_variant *vt);
bool		 rt_vt_array_append(rt_variant *vt, rt_variant *val);
rt_variant	*rt_vt_array_get(rt_variant *vt, size_t id);
size_t		 rt_vt_array_count(rt_variant *vt);
rt_variant	*rt_vt_object(rt_variant *vt);
bool		 rt_vt_object_set(rt_variant *vt, const char *name, rt_variant *val);
rt_variant	*rt_vt_object_get(rt_variant *vt, const char *name);
bool		 rt_vt_object_get_id(const rt_variant *vt, size_t id, const char **key, rt_variant **value);
bool		 rt_vt_object_remove(rt_variant *vt, const char *name);
size_t		 rt_vt_object_field_count(const rt_variant *vt);
bool		 rt_vt_object_has_field(rt_variant *vt, const char *name);
rt_variant	*rt_vt_clone(rt_variant *vt);
void		 rt_vt_copy(rt_variant *dst, const rt_variant *src);
void		 rt_vt_release(rt_variant *vt);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_RUNTIME_VARIANT_H_ */

