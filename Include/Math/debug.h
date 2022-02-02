/* NekoEngine
 *
 * debug.h
 * Author: Alexandru Naiman
 *
 * Math debugging functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2021, Alexandru Naiman
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
 */

#ifndef _NE_MATH_DEBUG_H_
#define _NE_MATH_DEBUG_H_

#include <System/Log.h>

#include <Math/defs.h>

#define MATH_LOG_OBJ(object, name, module) _Generic((object),	\
	struct vec2 *: v2_log(object, name, module),				\
	struct vec3 *: v3_log(object, name, module),				\
	struct vec4 *: v4_log(object, name, module),				\
	struct quat *: quat_log(object, name, module),				\
	struct mat3 *: m3_log(object, name, module),				\
	struct mat4 *: m4_log(object, name, module),				\
	struct ray2 *: r2_log(object, name, module),				\
	struct ray3 *: r3_log(object, name, module),				\
	struct aabb2 *: aabb2_log(object, name, module),			\
	struct aabb3 *: aabb3_log(object, name, module),			\
	struct plane *: plane_log(object, name, module))

static inline void
v2_log(const struct vec2 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: vec2(%.05f, %.05f)", name, v->x, v->y);
}

static inline void
v3_log(const struct vec3 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: vec3(%.05f, %.05f, %.05f)", name, v->x, v->y, v->z);
}

static inline void
v4_log(const struct vec4 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: vec4(%.05f, %.05f, %.05f, %.05f)", name, v->x, v->y, v->z, v->w);
}

static inline void
quat_log(const struct quat *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: quat(%f, %.05f, %.05f, %.05f)", name, v->x, v->y, v->z, v->w);
}

static inline void
m3_log(const struct mat3 *m, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: mat3\n\t%.05f, %.05f, %.05f\n\t%.05f, %.05f, %.05f\n\t%.05f, %.05f, %.05f", name,
		m->mat[0], m->mat[1], m->mat[2], m->mat[3], m->mat[4], m->mat[5], m->mat[6], m->mat[7], m->mat[8]);
}

static inline void
m4_log(const struct mat4 *m, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: mat4\n\t%.05f, %.05f, %.05f, %.05f\n\t%.05f, %.05f, %.05f, %.05f" \
		"\n\t%.05f, %.05f, %.05f, %.05f\n\t%.05f, %.05f, %.05f, %.05f", name,
		m->m[0], m->m[1], m->m[2], m->m[3], m->m[4], m->m[5], m->m[6], m->m[7], m->m[8], m->m[9], m->m[10],
		m->m[11], m->m[12], m->m[13], m->m[14], m->m[15]);
}

static inline void
r2_log(const struct ray2 *r, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: ray2(start = %.05f, %.05f; dir = %.05f, %.05f)", name,
		r->start.x, r->start.y, r->dir.x, r->dir.y);
}

static inline void
r3_log(const struct ray3 *r, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: ray3(start = %.05f, %.05f, %.05f; dir = %.05f, %.05f, %.05f)", name,
		r->start.x, r->start.y, r->start.z, r->dir.x, r->dir.y, r->dir.z);
}

static inline void
aabb2_log(const struct aabb2 *a, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: aabb2(min = %.05f, %.05f; max = %.05f, %.05f)", name,
		a->min.x, a->min.y, a->max.x, a->max.y);
}

static inline void
aabb3_log(const struct aabb3 *a, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: aabb3(min = %.05f, %.05f, %.05f; max = %.05f, %.05f, %.05f)", name,
		a->min.x, a->min.y, a->min.z, a->max.x, a->max.y, a->max.z);
}

static inline void
plane_log(const struct plane *p, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: p(%.05f, %.05f, %.05f, %.05f)", name, p->a, p->b, p->c, p->d);
}

#endif /* _NE_MATH_DEBUG_H_ */
