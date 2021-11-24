/* NekoEngine
 *
 * mat4.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions
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
 * -----------------------------------------------------------------------------
 * Original copyright:

Copyright (c) 2008, Luke Benstead.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _NE_MATH_MAT4_H_
#define _NE_MATH_MAT4_H_

#include <Math/defs.h>

#if defined(USE_SSE)
#	include <Math/sse/mat4_sse.h>
#elif defined(USE_ALTIVEC)
#	include <Math/altivec/mat4_altivec.h>
#elif defined(USE_NEON)
#	include <Math/neon/mat4_neon.h>
#elif defined(USE_VMX128)
#	include <Math/vmx128/mat4_vmx128.h>
#endif

#include <Math/vec3.h>
#include <Math/vec4.h>
#include <Math/quat.h>

#ifdef MATH_SIMD

#define m4				m4_simd
#define m4f				m4f_simd
#define m4_copy			m4_copy_simd
#define m4_mul			m4_mul_simd
#define m4_muls			m4_muls_simd
#define v4_mul_m4		v4_mul_m4_simd

#ifndef M4_TRANSPOSE_NOSIMD
#define m4_transpose	m4_transpose_simd
#endif

#ifndef M4_INVERSE_NOSIMD
#define m4_inverse		m4_inverse_simd
#endif

#else

static inline struct mat4 *
m4(struct mat4 *dst, const float *m)
{
	memcpy(dst->m, m, sizeof(float) * 16);
	return dst;
}

static inline struct mat4 *
m4f(struct mat4 *dst,
	float m0, float m1, float m2, float m3,
	float m4, float m5, float m6, float m7,
	float m8, float m9, float m10, float m11,
	float m12, float m13, float m14, float m15)
{
	dst->m[0] = m0; dst->m[1] = m1; dst->m[2] = m2; dst->m[3] = m3;
	dst->m[4] = m4; dst->m[5] = m5; dst->m[6] = m6; dst->m[7] = m7;
	dst->m[8] = m8; dst->m[9] = m9; dst->m[10] = m10; dst->m[11] = m11;
	dst->m[12] = m12; dst->m[13] = m13; dst->m[14] = m14; dst->m[15] = m15;
	
	return dst;
}

static inline struct mat4 *
m4_copy(struct mat4 *dst, const struct mat4 *src)
{
	memcpy(dst->m, src->m, sizeof(float) * 16);
	return dst;
}

static inline struct mat4 *
m4_mul(struct mat4 *dst, const struct mat4 *m1, const struct mat4 *m2)
{
	const float mat[16] =
	{
		m1->m[0] * m2->m[0] + m1->m[4] * m2->m[1] + m1->m[8]  * m2->m[2]  + m1->m[12] * m2->m[3],
		m1->m[1] * m2->m[0] + m1->m[5] * m2->m[1] + m1->m[9]  * m2->m[2]  + m1->m[13] * m2->m[3],
		m1->m[2] * m2->m[0] + m1->m[6] * m2->m[1] + m1->m[10] * m2->m[2]  + m1->m[14] * m2->m[3],
		m1->m[3] * m2->m[0] + m1->m[7] * m2->m[1] + m1->m[11] * m2->m[2]  + m1->m[15] * m2->m[3],

		m1->m[0] * m2->m[4] + m1->m[4] * m2->m[5] + m1->m[8]  * m2->m[6]  + m1->m[12] * m2->m[7],
		m1->m[1] * m2->m[4] + m1->m[5] * m2->m[5] + m1->m[9]  * m2->m[6]  + m1->m[13] * m2->m[7],
		m1->m[2] * m2->m[4] + m1->m[6] * m2->m[5] + m1->m[10] * m2->m[6]  + m1->m[14] * m2->m[7],
		m1->m[3] * m2->m[4] + m1->m[7] * m2->m[5] + m1->m[11] * m2->m[6]  + m1->m[15] * m2->m[7],

		m1->m[0] * m2->m[8] + m1->m[4] * m2->m[9] + m1->m[8]  * m2->m[10] + m1->m[12] * m2->m[11],
		m1->m[1] * m2->m[8] + m1->m[5] * m2->m[9] + m1->m[9]  * m2->m[10] + m1->m[13] * m2->m[11],
		m1->m[2] * m2->m[8]  + m1->m[6] * m2->m[9]  + m1->m[10] * m2->m[10] + m1->m[14] * m2->m[11],
		m1->m[3] * m2->m[8]  + m1->m[7] * m2->m[9]  + m1->m[11] * m2->m[10] + m1->m[15] * m2->m[11],

		m1->m[0] * m2->m[12] + m1->m[4] * m2->m[13] + m1->m[8]  * m2->m[14] + m1->m[12] * m2->m[15],
		m1->m[1] * m2->m[12] + m1->m[5] * m2->m[13] + m1->m[9]  * m2->m[14] + m1->m[13] * m2->m[15],
		m1->m[2] * m2->m[12] + m1->m[6] * m2->m[13] + m1->m[10] * m2->m[14] + m1->m[14] * m2->m[15],
		m1->m[3] * m2->m[12] + m1->m[7] * m2->m[13] + m1->m[11] * m2->m[14] + m1->m[15] * m2->m[15],
	};
	
	memcpy(dst->m, mat, sizeof(float) * 16);

	return dst;
}

static inline struct mat4 *
m4_muls(struct mat4 *dst, const struct mat4 *m, const float f)
{
	dst->m[0] = m->m[0] * f;
	dst->m[1] = m->m[1] * f;
	dst->m[2] = m->m[2] * f;
	dst->m[3] = m->m[3] * f;

	dst->m[4] = m->m[4] * f;
	dst->m[5] = m->m[5] * f;
	dst->m[6] = m->m[6] * f;
	dst->m[7] = m->m[7] * f;

	dst->m[8] = m->m[8] * f;
	dst->m[9] = m->m[9] * f;
	dst->m[10] = m->m[10] * f;
	dst->m[11] = m->m[11] * f;

	dst->m[12] = m->m[12] * f;
	dst->m[13] = m->m[13] * f;
	dst->m[14] = m->m[14] * f;
	dst->m[15] = m->m[15] * f;

	return dst;
}

#endif

#if !defined(MATH_SIMD) || defined(M4_INVERSE_NOSIMD)
static inline struct mat4 *
m4_inverse(struct mat4 *dst, const struct mat4 *src)
{
	struct mat4 tmp;
	float det = 0.f;
	int i = 0;

	tmp.m[0] = src->m[5] * src->m[10] * src->m[15] -
		src->m[5] * src->m[11] * src->m[14] -
		src->m[9] * src->m[6] * src->m[15] +
		src->m[9] * src->m[7] * src->m[14] +
		src->m[13] * src->m[6] * src->m[11] -
		src->m[13] * src->m[7] * src->m[10];

	tmp.m[4] = -src->m[4] * src->m[10] * src->m[15] +
		src->m[4] * src->m[11] * src->m[14] +
		src->m[8] * src->m[6] * src->m[15] -
		src->m[8] * src->m[7] * src->m[14] -
		src->m[12] * src->m[6] * src->m[11] +
		src->m[12] * src->m[7] * src->m[10];

	tmp.m[8] = src->m[4] * src->m[9] * src->m[15] -
		src->m[4] * src->m[11] * src->m[13] -
		src->m[8] * src->m[5] * src->m[15] +
		src->m[8] * src->m[7] * src->m[13] +
		src->m[12] * src->m[5] * src->m[11] -
		src->m[12] * src->m[7] * src->m[9];

	tmp.m[12] = -src->m[4] * src->m[9] * src->m[14] +
		src->m[4] * src->m[10] * src->m[13] +
		src->m[8] * src->m[5] * src->m[14] -
		src->m[8] * src->m[6] * src->m[13] -
		src->m[12] * src->m[5] * src->m[10] +
		src->m[12] * src->m[6] * src->m[9];

	tmp.m[1] = -src->m[1] * src->m[10] * src->m[15] +
		src->m[1] * src->m[11] * src->m[14] +
		src->m[9] * src->m[2] * src->m[15] -
		src->m[9] * src->m[3] * src->m[14] -
		src->m[13] * src->m[2] * src->m[11] +
		src->m[13] * src->m[3] * src->m[10];

	tmp.m[5] = src->m[0] * src->m[10] * src->m[15] -
		src->m[0] * src->m[11] * src->m[14] -
		src->m[8] * src->m[2] * src->m[15] +
		src->m[8] * src->m[3] * src->m[14] +
		src->m[12] * src->m[2] * src->m[11] -
		src->m[12] * src->m[3] * src->m[10];

	tmp.m[9] = -src->m[0] * src->m[9] * src->m[15] +
		src->m[0] * src->m[11] * src->m[13] +
		src->m[8] * src->m[1] * src->m[15] -
		src->m[8] * src->m[3] * src->m[13] -
		src->m[12] * src->m[1] * src->m[11] +
		src->m[12] * src->m[3] * src->m[9];

	tmp.m[13] = src->m[0] * src->m[9] * src->m[14] -
		src->m[0] * src->m[10] * src->m[13] -
		src->m[8] * src->m[1] * src->m[14] +
		src->m[8] * src->m[2] * src->m[13] +
		src->m[12] * src->m[1] * src->m[10] -
		src->m[12] * src->m[2] * src->m[9];

	tmp.m[2] = src->m[1] * src->m[6] * src->m[15] -
		src->m[1] * src->m[7] * src->m[14] -
		src->m[5] * src->m[2] * src->m[15] +
		src->m[5] * src->m[3] * src->m[14] +
		src->m[13] * src->m[2] * src->m[7] -
		src->m[13] * src->m[3] * src->m[6];

	tmp.m[6] = -src->m[0] * src->m[6] * src->m[15] +
		src->m[0] * src->m[7] * src->m[14] +
		src->m[4] * src->m[2] * src->m[15] -
		src->m[4] * src->m[3] * src->m[14] -
		src->m[12] * src->m[2] * src->m[7] +
		src->m[12] * src->m[3] * src->m[6];

	tmp.m[10] = src->m[0] * src->m[5] * src->m[15] -
		src->m[0] * src->m[7] * src->m[13] -
		src->m[4] * src->m[1] * src->m[15] +
		src->m[4] * src->m[3] * src->m[13] +
		src->m[12] * src->m[1] * src->m[7] -
		src->m[12] * src->m[3] * src->m[5];

	tmp.m[14] = -src->m[0] * src->m[5] * src->m[14] +
		src->m[0] * src->m[6] * src->m[13] +
		src->m[4] * src->m[1] * src->m[14] -
		src->m[4] * src->m[2] * src->m[13] -
		src->m[12] * src->m[1] * src->m[6] +
		src->m[12] * src->m[2] * src->m[5];

	tmp.m[3] = -src->m[1] * src->m[6] * src->m[11] +
		src->m[1] * src->m[7] * src->m[10] +
		src->m[5] * src->m[2] * src->m[11] -
		src->m[5] * src->m[3] * src->m[10] -
		src->m[9] * src->m[2] * src->m[7] +
		src->m[9] * src->m[3] * src->m[6];

	tmp.m[7] = src->m[0] * src->m[6] * src->m[11] -
		src->m[0] * src->m[7] * src->m[10] -
		src->m[4] * src->m[2] * src->m[11] +
		src->m[4] * src->m[3] * src->m[10] +
		src->m[8] * src->m[2] * src->m[7] -
		src->m[8] * src->m[3] * src->m[6];

	tmp.m[11] = -src->m[0] * src->m[5] * src->m[11] +
		src->m[0] * src->m[7] * src->m[9] +
		src->m[4] * src->m[1] * src->m[11] -
		src->m[4] * src->m[3] * src->m[9] -
		src->m[8] * src->m[1] * src->m[7] +
		src->m[8] * src->m[3] * src->m[5];

	tmp.m[15] = src->m[0] * src->m[5] * src->m[10] -
		src->m[0] * src->m[6] * src->m[9] -
		src->m[4] * src->m[1] * src->m[10] +
		src->m[4] * src->m[2] * src->m[9] +
		src->m[8] * src->m[1] * src->m[6] -
		src->m[8] * src->m[2] * src->m[5];

	det = src->m[0] *
		tmp.m[0] + src->m[1] *
		tmp.m[4] + src->m[2] *
		tmp.m[8] + src->m[3] *
		tmp.m[12];

	if (det == 0)
		return NULL;

	det = 1.f / det;

	for (i = 0; i < 16; i++)
		dst->m[i] = tmp.m[i] * det;

	return dst;
}
#endif

#if !defined(MATH_SIMD) || defined(M4_TRANSPOSE_NOSIMD)
static inline struct mat4 *
m4_transpose(struct mat4 *dst, const struct mat4 *src)
{
	struct mat4 tmp;
	int x, z;

	for (z = 0; z < 4; ++z)
		for (x = 0; x < 4; ++x)
			tmp.m[(z * 4) + x] = src->m[(x * 4) + z];

	memcpy(dst, &tmp, sizeof(*dst));

	return dst;
}
#endif

#if !defined(MATH_SIMD)
static inline struct vec4 *
v4_mul_m4(struct vec4 *dst, const struct vec4 *v, const struct mat4 *m)
{
	dst->x = v->x * m->m[0] + v->y * m->m[1] + v->z * m->m[2] + v->w * m->m[3];
	dst->y = v->x * m->m[4] + v->y * m->m[5] + v->z * m->m[6] + v->w * m->m[7];
	dst->z = v->x * m->m[8] + v->y * m->m[9] + v->z * m->m[10] + v->w * m->m[11];
	dst->w = v->x * m->m[12] + v->y * m->m[13] + v->z * m->m[14] + v->w * m->m[15];

	return dst;
}
#endif

static inline struct mat4 *
m4_ident(struct mat4 *m)
{
	float md[16] = { 0.f };
	md[0] = md[5] = md[10] = md[15] = 1.f;
	return m4(m, md);
}

static inline struct mat4 *
m4_init_m3(struct mat4 *dst, const struct mat3 *src)
{
	return m4f(dst,
		src->mat[0], src->mat[1], src->mat[2], 0.f,
		src->mat[3], src->mat[4], src->mat[5], 0.f,
		src->mat[6], src->mat[7], src->mat[8], 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct mat4 *
m4_rot_x(struct mat4 *dst, const float rad)
{
	return m4f(dst,
		1.f, 0.f, 0.f, 0.f,
		0.f, cosf(rad), sinf(rad), 0.f,
		0.f, -sinf(rad), cosf(rad), 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct mat4 *
m4_rot_y(struct mat4 *dst, const float rad)
{
	return m4f(dst,
		cosf(rad), 0.f, -sinf(rad), 0.f,
		0.f, 1.f, 0.f, 0.f,
		sinf(rad), 0.f, cosf(rad), 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct mat4 *
m4_rot_z(struct mat4 *dst, const float rad)
{
	return m4f(dst,
		cosf(rad), sinf(rad), 0.f, 0.f,
		-sinf(rad), cosf(rad), 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct mat4 *
m4_rot_quat(struct mat4 *dst, const struct quat *q)
{
	const float xx = q->x * q->x;
	const float xy = q->x * q->y;
	const float xz = q->x * q->z;
	const float xw = q->x * q->w;

	const float yy = q->y * q->y;
	const float yz = q->y * q->z;
	const float yw = q->y * q->w;

	const float zz = q->z * q->z;
	const float zw = q->z * q->w;

	return m4f(dst,
		1.f - 2.f * (yy + zz), 2.f * (xy + zw), 2.f * (xz - yw), 0.f,
		2.f * (xy - zw), 1.f - 2.f * (xx + zz), 2.f * (yz + xw), 0.f,
		2.f * (xz + yw), 2.f * (yz - xw), 1.f - 2.f * (xx + yy), 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct mat4 *
m4_rot_axis_angle(struct mat4 *dst, const struct vec3 *axis, float deg)
{
	struct quat quat;
	quat_rot_axis_angle(&quat, axis, deg);
	m4_rot_quat(dst, &quat);
	return dst;
}

static inline struct mat4 *
m4_rot_pitch_yaw_roll(struct mat4 *dst, const float pitch, const float yaw, const float roll)
{
	struct mat4 yaw_matrix;
	struct mat4 roll_matrix;
	struct mat4 pitch_matrix;

	m4_rot_y(&yaw_matrix, yaw);
	m4_rot_x(&pitch_matrix, pitch);
	m4_rot_z(&roll_matrix, roll);

	m4_mul(dst, &pitch_matrix, &roll_matrix);
	m4_mul(dst, &yaw_matrix, dst);

	return dst;
}

static inline struct mat4 *
m4_look_at(struct mat4 *dst, const struct vec3 *eye, const struct vec3 *center, const struct vec3 *up)
{
	struct vec3 f;
	struct vec3 s;
	struct vec3 u;

	v3_sub(&f, center, eye);
	v3_norm(&f, &f);

	v3_cross(&s, &f, up);
	v3_norm(&s, &s);

	v3_cross(&u, &s, &f);

	return m4f(dst,
		s.x, u.x, -f.x, 0.f,
		s.y, u.y, -f.y, 0.f,
		s.z, u.z, -f.z, 0.f,
		-v3_dot(&s, eye), -v3_dot(&u, eye), v3_dot(&f, eye), 1.f);
}


static inline struct mat4 *
m4_scale(struct mat4 *dst, const float x, const float y, const float z)
{
	return m4f(dst,
		  x, 0.f, 0.f, 0.f,
		0.f,   y, 0.f, 0.f,
		0.f, 0.f,   z, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct mat4 *
m4_scale_v(struct mat4 *dst, const struct vec3 *v)
{
	return m4_scale(dst, v->x, v->y, v->z);
}

static inline struct mat4 *
m4_translate(struct mat4 *dst, const float x, const float y, const float z)
{
	return m4f(dst,
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		  x,   y,   z, 1.f);
}

static inline struct mat4 *
m4_translate_v(struct mat4 *dst, const struct vec3 *v)
{
	return m4_translate(dst, v->x, v->y, v->z);
}

static inline struct vec3 *
m4_up(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &v3_pos_y, m);
	return v3_norm(v, v);
}

static inline struct vec3 *
m4_fwd_rh(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &v3_neg_z, m);
	return v3_norm(v, v);
}

static inline struct vec3 *
m4_fwd_lh(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &v3_pos_z, m);
	return v3_norm(v, v);
}

static inline struct vec3 *
m4_right(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &v3_pos_x, m);
	return v3_norm(v, v);
}

static inline struct mat4 *
m4_perspective(struct mat4 *dst, float fov_y, float aspect, float z_near, float z_far)
{
	const float rad = 0.5f * deg_to_rad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);
	const float w = h / aspect;

	const float m22 = z_far / (z_near - z_far);
	const float m32 = -(z_far * z_near) / (z_far - z_near);

	return m4f(dst,
		  w, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, m22, -1.f,
		0.f, 0.f, m32,  1.f);
}

static inline struct mat4 *
m4_perspective_nd(struct mat4 *dst, float fov_y, float aspect, float z_near, float z_far)
{
	const float rad = 0.5f * deg_to_rad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);
	const float w = h / aspect;

	const float m22 = -(z_far * z_near) / (z_far - z_near);
	const float m32 = -(2.f * z_far * z_near) / (z_far - z_near);

	return m4f(dst,
		  w, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, m22, -1.f,
		0.f, 0.f, m32,  1.f);
}

static inline struct mat4 *
m4_infinite_perspective_rz(struct mat4 *dst, float fov_y, float aspect, float z_near)
{
	const float rad = 0.5f * deg_to_rad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);

	return m4f(dst,
		h / aspect, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, 0.f, -1.f,
		0.f, 0.f, z_near,  0.f);
}

static inline struct mat4 *
m4_ortho(struct mat4 *dst, float left, float right, float bottom, float top, float z_near, float z_far)
{
	return m4f(dst,
		2.f / (right - left), 0.f, 0.f,  0.f,
		0.f, 2.f / (top - bottom), 0.f,  0.f,
		0.f, 0.f, 1.f / (z_far - z_near), 0.f,
		-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -dst->r[2][2] * z_near, 1.f);
}

static inline struct mat4 *
m4_ortho_nd(struct mat4 *dst, float left, float right, float bottom, float top, float z_near, float z_far)
{
	return m4f(dst,
		2.f / (right - left), 0.f, 0.f,  0.f,
		0.f, 2.f / (top - bottom), 0.f,  0.f,
		0.f, 0.f, -2.f / (z_far - z_near), 0.f,
		-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((z_far + z_near) / (z_far - z_near)), 1.f);
}

#endif /* _NE_MATH_MAT4_H_ */
