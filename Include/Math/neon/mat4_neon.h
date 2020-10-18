/* NekoEngine
 *
 * mat4_neon.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions NEON implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2020, Alexandru Naiman
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

#ifndef _NE_MATH_NEON_MAT4_H_
#define _NE_MATH_NEON_MAT4_H_

#include <Math/defs.h>

#ifdef USE_NEON

#include <Math/vec3.h>
#include <Math/quat.h>

static inline struct mat4 *
m4(struct mat4 *dst, const float *m)
{
	dst->sm[0] = vld1q_f32(&m[0]);
	dst->sm[1] = vld1q_f32(&m[4]);
	dst->sm[2] = vld1q_f32(&m[8]);
	dst->sm[3] = vld1q_f32(&m[12]);
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
	dst->sm[0] = vld1q_f32(&src->r[0][0]);
	dst->sm[1] = vld1q_f32(&src->r[1][0]);
	dst->sm[2] = vld1q_f32(&src->r[2][0]);
	dst->sm[3] = vld1q_f32(&src->r[3][0]);

	return dst;
}

static inline struct mat4 *
m4_ident(struct mat4 *m)
{
	memset(m->m, 0, sizeof(float) * 16);
	m->m[0] = m->m[5] = m->m[10] = m->m[15] = 1.f;
	return m;
}

static inline struct mat4 *
m4_init_m3(struct mat4 *m4, const struct mat3 *m3)
{
	m4_ident(m4);
	
	m4->m[0] = m3->mat[0];
	m4->m[1] = m3->mat[1];
	m4->m[2] = m3->mat[2];
	m4->m[3] = 0.0;
	
	m4->m[4] = m3->mat[3];
	m4->m[5] = m3->mat[4];
	m4->m[6] = m3->mat[5];
	m4->m[7] = 0.0;
	
	m4->m[8] = m3->mat[6];
	m4->m[9] = m3->mat[7];
	m4->m[10] = m3->mat[8];
	m4->m[11] = 0.0;
	
	m4->m[15] = 1.f;
	
	return m4;
}

static inline struct mat4 *
m4_mul(struct mat4 *dst, const struct mat4 *m1, const struct mat4 *m2)
{
    float32x4_t zero = vmovq_n_f32(0.f);
    float32x4_t r0, r1, r2, r3;

	// TODO: profile instruction order
	r0 = vmlaq_f32(zero, vdupq_n_f32(m2->r[0][0]), m1->sm[0]);
	r1 = vmlaq_f32(zero, vdupq_n_f32(m2->r[1][0]), m1->sm[0]);
	r2 = vmlaq_f32(zero, vdupq_n_f32(m2->r[2][0]), m1->sm[0]);
	r3 = vmlaq_f32(zero, vdupq_n_f32(m2->r[3][0]), m1->sm[0]);

	r0 = vmlaq_f32(r0, vdupq_n_f32(m2->r[0][1]), m1->sm[1]);
	r1 = vmlaq_f32(r1, vdupq_n_f32(m2->r[1][1]), m1->sm[1]);
	r2 = vmlaq_f32(r2, vdupq_n_f32(m2->r[2][1]), m1->sm[1]);
	r3 = vmlaq_f32(r3, vdupq_n_f32(m2->r[3][1]), m1->sm[1]);
	
	r0 = vmlaq_f32(r0, vld1q_dup_f32(&m2->r[0][2]), m1->sm[2]);
	r1 = vmlaq_f32(r1, vld1q_dup_f32(&m2->r[1][2]), m1->sm[2]);
	r2 = vmlaq_f32(r2, vld1q_dup_f32(&m2->r[2][2]), m1->sm[2]);
	r3 = vmlaq_f32(r3, vld1q_dup_f32(&m2->r[3][2]), m1->sm[2]);
	
	r0 = vmlaq_f32(r0, vld1q_dup_f32(&m2->r[0][3]), m1->sm[3]);
	r1 = vmlaq_f32(r1, vld1q_dup_f32(&m2->r[1][3]), m1->sm[3]);
	r2 = vmlaq_f32(r2, vld1q_dup_f32(&m2->r[2][3]), m1->sm[3]);
	r3 = vmlaq_f32(r3, vld1q_dup_f32(&m2->r[3][3]), m1->sm[3]);
	
	dst->sm[0] = r0;
	dst->sm[1] = r1;
	dst->sm[2] = r2;
	dst->sm[3] = r3;
	
	return dst;
}

static inline struct mat4 *
m4_mul_scalar(struct mat4 *dst, const struct mat4 *m, const float f)
{
	float32x4_t scalar = vdupq_n_f32(f);

	dst->sm[0] = vmulq_f32(m->sm[0], scalar);
	dst->sm[1] = vmulq_f32(m->sm[1], scalar);
	dst->sm[2] = vmulq_f32(m->sm[2], scalar);
	dst->sm[3] = vmulq_f32(m->sm[3], scalar);
	
	return dst;
}

static inline struct mat4 *
m4_transpose(struct mat4 *dst, const struct mat4 *src)
{	
	const float32x4_t r0 = vtrn1q_f32(src->sm[0], src->sm[1]);
	const float32x4_t r1 = vtrn2q_f32(src->sm[0], src->sm[1]);
	const float32x4_t r2 = vtrn1q_f32(src->sm[2], src->sm[3]);
	const float32x4_t r3 = vtrn2q_f32(src->sm[2], src->sm[3]);

	dst->sm[0] = vcombine_f32(vget_low_f32(r0), vget_low_f32(r2));
	dst->sm[1] = vcombine_f32(vget_low_f32(r1), vget_low_f32(r3));
	dst->sm[2] = vcombine_f32(vget_high_f32(r0), vget_high_f32(r2));
	dst->sm[3] = vcombine_f32(vget_high_f32(r1), vget_high_f32(r3));
	
	return dst;
}

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

static inline struct mat4 *
m4_rot_x(struct mat4 *dst, const float radians)
{
	dst->m[0] = 1.f;
	dst->m[1] = 0.f;
	dst->m[2] = 0.f;
	dst->m[3] = 0.f;

	dst->m[4] = 0.f;
	dst->m[5] = cosf(radians);
	dst->m[6] = sinf(radians);
	dst->m[7] = 0.f;
	
	dst->m[8] = 0.f;
	dst->m[9] = -sinf(radians);
	dst->m[10] = cosf(radians);
	dst->m[11] = 0.f;
	
	dst->m[12] = 0.f;
	dst->m[13] = 0.f;
	dst->m[14] = 0.f;
	dst->m[15] = 1.f;

	return dst;
}

static inline struct mat4 *
m4_rot_y(struct mat4 *dst, const float radians)
{
	dst->m[0] = cosf(radians);
	dst->m[1] = 0.f;
	dst->m[2] = -sinf(radians);
	dst->m[3] = 0.f;
	
	dst->m[4] = 0.f;
	dst->m[5] = 1.f;
	dst->m[6] = 0.f;
	dst->m[7] = 0.f;
	
	dst->m[8] = sinf(radians);
	dst->m[9] = 0.f;
	dst->m[10] = cosf(radians);
	dst->m[11] = 0.f;

	dst->m[12] = 0.f;
	dst->m[13] = 0.f;
	dst->m[14] = 0.f;
	dst->m[15] = 1.f;

	return dst;
}

static inline struct mat4 *
m4_rot_z(struct mat4 *dst, const float radians)
{
	dst->m[0] = cosf(radians);
	dst->m[1] = sinf(radians);
	dst->m[2] = 0.f;
	dst->m[3] = 0.f;

	dst->m[4] = -sinf(radians);
	dst->m[5] = cosf(radians);
	dst->m[6] = 0.f;
	dst->m[7] = 0.f;

	dst->m[8] = 0.f;
	dst->m[9] = 0.f;
	dst->m[10] = 1.f;
	dst->m[11] = 0.f;

	dst->m[12] = 0.f;
	dst->m[13] = 0.f;
	dst->m[14] = 0.f;
	dst->m[15] = 1.f;

	return dst;
}

static inline struct mat4 *
m4_rot_quat(struct mat4 *dst, const struct quat *q)
{
	// TODO: vectorize
	float xx = q->x * q->x;
	float xy = q->x * q->y;
	float xz = q->x * q->z;
	float xw = q->x * q->w;
	
	float yy = q->y * q->y;
	float yz = q->y * q->z;
	float yw = q->y * q->w;
	
	float zz = q->z * q->z;
	float zw = q->z * q->w;

	dst->m[0] = 1.f - 2.f * (yy + zz);
	dst->m[1] = 2.f * (xy + zw);
	dst->m[2] = 2.f * (xz - yw);
	dst->m[3] = 0.f;
	
	dst->m[4] = 2.f * (xy - zw);
	dst->m[5] = 1.f - 2.f * (xx + zz);
	dst->m[6] = 2.f * (yz + xw);
	dst->m[7] = 0.f;
	
	dst->m[8] = 2.f * (xz + yw);
	dst->m[9] = 2.f * (yz - xw);
	dst->m[10] = 1.f - 2.f * (xx + yy);
	dst->m[11] = 0.f;

	dst->m[12] = 0.f;
	dst->m[13] = 0.f;
	dst->m[14] = 0.f;
	dst->m[15] = 1.f;

	return dst;
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

	dst->m[0] = s.x;
	dst->m[1] = u.x;
	dst->m[2] = -f.x;
	dst->m[3] = 0.f;
	
	dst->m[4] = s.y;
	dst->m[5] = u.y;
	dst->m[6] = -f.y;
	dst->m[7] = 0.f;
	
	dst->m[8] = s.z;
	dst->m[9] = u.z;
	dst->m[10] = -f.z;
	dst->m[11] = 0.f;
	
	dst->m[12] = -v3_dot(&s, eye);
	dst->m[13] = -v3_dot(&u, eye);
	dst->m[14] = v3_dot(&f, eye);
	dst->m[15] = 1.f;

	return dst;
}

static inline struct mat4 *
m4_scale(struct mat4 *dst, const float x, const float y, const float z)
{
	memset(dst->m, 0, sizeof(float) * 16);
	
	dst->m[0] = x;
	dst->m[5] = y;
	dst->m[10] = z;
	dst->m[15] = 1.f;

	return dst;
}

static inline struct mat4 *
m4_scale_v(struct mat4 *dst, struct vec3 *v)
{
	return m4_scale(dst, v->x, v->y, v->z);
}

static inline struct mat4 *
m4_translate(struct mat4 *dst, const float x, const float y, const float z)
{
	memset(dst->m, 0, sizeof(float) * 16);

	dst->m[0] = 1.f;
	dst->m[5] = 1.f;
	dst->m[10] = 1.f;

	dst->m[12] = x;
	dst->m[13] = y;
	dst->m[14] = z;
	dst->m[15] = 1.f;

	return dst;
}

static inline struct mat4 *
m4_translate_v(struct mat4 *dst, struct vec3 *v)
{
	return m4_translate(dst, v->x, v->y, v->z);
}

static inline struct vec3 *
m4_up(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &KM_VEC3_POS_Y, m);
	return v3_norm(v, v);
}

static inline struct vec3 *
m4_fwd_rh(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &KM_VEC3_NEG_Z, m);
	return v3_norm(v, v);
}

static inline struct vec3 *
m4_fwd_lh(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &KM_VEC3_POS_Z, m);
	return v3_norm(v, v);
}

static inline struct vec3 *
m4_right(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &KM_VEC3_POS_X, m);
	return v3_norm(v, v);
}

static inline struct mat4 *
m4_perspective(struct mat4 *dst, float fov_y, float aspect, float near, float far)
{
	const float rad = 0.5f * deg_to_rad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);
	const float w = h / aspect;
	
	memset(dst, 0x0, sizeof(*dst));
	
	dst->r[0][0] = w;
	dst->r[1][1] = h;
	dst->r[2][2] = far / (near - far);
	dst->r[2][3] = -1.0f;
	dst->r[3][2] = -(far * near) / (far - near);
	dst->r[3][3] = 1.f;
	
	return dst;
}

static inline struct mat4 *
m4_perspective_nd(struct mat4 *dst, float fov_y, float aspect, float near, float far)
{
	const float rad = 0.5f * deg_to_rad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);
	const float w = h / aspect;
	
	memset(dst, 0x0, sizeof(*dst));
	
	dst->r[0][0] = w;
	dst->r[1][1] = h;
	dst->r[2][2] = -(far * near) / (far - near);
	dst->r[2][3] = -1.0f;
	dst->r[3][2] = -(2.f * far * near) / (far - near);
	dst->r[3][3] = 1.f;
	
	return dst;
}

static inline struct mat4 *
m4_infinite_perspective_rz(struct mat4 *dst, float fov_y, float aspect, float near)
{
	const float f = 1.f / tanf(deg_to_rad(fov_y) / 2.f);
	
	memset(dst, 0x0, sizeof(*dst));
	
	dst->r[0][0] = f / aspect;
	dst->r[1][1] = f;
	dst->r[2][2] = 1.f;
	dst->r[2][3] = -1.f;
	dst->r[3][2] = near;
	dst->r[3][3] = 1.f;
	
	return dst;
}

static inline struct mat4 *
m4_ortho(struct mat4 *dst, float left, float right, float bottom, float top, float z_near, float z_far)
{
	memset(dst, 0x0, sizeof(*dst));
	
	dst->r[0][0] = 2.f / (right - left);
	dst->r[1][1] = 2.f / (top - bottom);
	dst->r[2][2] = 1.f / (z_far - z_near);
	dst->r[3][0] = -((right + left) / (right - left));
	dst->r[3][1] = -((top + bottom) / (top - bottom));
	dst->r[3][2] = -dst->r[2][2] * z_near;
	dst->r[3][3] = 1.f;
	
	return dst;
}

static inline struct mat4 *
m4_ortho_nd(struct mat4 *dst, float left, float right, float bottom, float top, float z_near, float z_far)
{
	memset(dst, 0x0, sizeof(*dst));
	
	dst->r[0][0] = 2.f / (right - left);
	dst->r[1][1] = 2.f / (top - bottom);
	dst->r[2][2] = -2.f / (z_far - z_near);
	dst->r[3][0] = -((right + left) / (right - left));
	dst->r[3][1] = -((top + bottom) / (top - bottom));
	dst->r[3][2] = -((z_far + z_near) / (z_far - z_near));
	dst->r[3][3] = 1.f;
	
	return dst;
}

#endif

#endif /* _NE_MATH_NEON_MAT4_H_ */
