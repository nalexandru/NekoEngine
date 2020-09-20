/* NekoEngine
 *
 * mat3.h
 * Author: Alexandru Naiman
 *
 * 3x3 matrix functions
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

#ifndef _NE_MATH_MAT3_H_
#define _NE_MATH_MAT3_H_

#include <stdlib.h>
#include <string.h>

#include <Math/defs.h>
#include <Math/vec3.h>

static inline struct mat3 *
m3(
	struct mat3 *dst,
	const float *m)
{
	memcpy(dst->mat, m, sizeof(float) * 9);
	return dst;
}

static inline struct mat3 *
m3f(
	struct mat3 *dst,
	float m0, float m1, float m2,
	float m3, float m4, float m5,
	float m6, float m7, float m8)
{
	dst->mat[0] = m0; dst->mat[1] = m1; dst->mat[2] = m2;
	dst->mat[3] = m3; dst->mat[4] = m4; dst->mat[5] = m5;
	dst->mat[6] = m6; dst->mat[7] = m7; dst->mat[8] = m8;

	return dst;
}

/*
 * Assigns the value of pIn to pOut
 */
static inline struct mat3 *
m3_copy(
	struct mat3 *dst,
	const struct mat3 *src)
{
	memcpy(dst->mat, src->mat, sizeof(float) * 9);
	return dst;
}

static inline struct mat3 *
m3_ident(struct mat3 *dst)
{
	memset(dst->mat, 0, sizeof(float) * 9);
	dst->mat[0] = dst->mat[4] = dst->mat[8] = 1.f;
	return dst;
}

static inline float
m3_det(const struct mat3 *m)
{
	float output;

	/*
	 * calculating the determinant following the rule of sarus,
	 *		| 0  3  6 | 0  3 |
	 *	m =	| 1  4  7 | 1  4 |
	 *		| 2  5  8 | 2  5 |
	 * now sum up the products of the diagonals going to the right (i.e. 0,4,8)
	 * and substract the products of the other diagonals (i.e. 2,4,6)
	 */

	output = m->mat[0] *
		m->mat[4] *
		m->mat[8] + m->mat[1] *
		m->mat[5] *
		m->mat[6] + m->mat[2] *
		m->mat[3] * m->mat[7];

	output -= m->mat[2] *
		m->mat[4] *
		m->mat[6] + m->mat[0] *
		m->mat[5] *
		m->mat[7] + m->mat[1] *
		m->mat[3] * m->mat[8];

	return output;
}

/*
 * Multiplies pM1 with pM2, stores the result in pOut, returns pOut
 */
static inline struct mat3 *
m3_mul(
	struct mat3 *dst,
	const struct mat3 *m1,
	const struct mat3 *m2)
{
	float mat[9];

	mat[0] = m1->mat[0] * m2->mat[0] + m1->mat[3] * m2->mat[1] + m1->mat[6] * m2->mat[2];
	mat[1] = m1->mat[1] * m2->mat[0] + m1->mat[4] * m2->mat[1] + m1->mat[7] * m2->mat[2];
	mat[2] = m1->mat[2] * m2->mat[0] + m1->mat[5] * m2->mat[1] + m1->mat[8] * m2->mat[2];
	mat[3] = m1->mat[0] * m2->mat[3] + m1->mat[3] * m2->mat[4] + m1->mat[6] * m2->mat[5];
	mat[4] = m1->mat[1] * m2->mat[3] + m1->mat[4] * m2->mat[4] + m1->mat[7] * m2->mat[5];
	mat[5] = m1->mat[2] * m2->mat[3] + m1->mat[5] * m2->mat[4] + m1->mat[8] * m2->mat[5];
	mat[6] = m1->mat[0] * m2->mat[6] + m1->mat[3] * m2->mat[7] + m1->mat[6] * m2->mat[8];
	mat[7] = m1->mat[1] * m2->mat[6] + m1->mat[4] * m2->mat[7] + m1->mat[7] * m2->mat[8];
	mat[8] = m1->mat[2] * m2->mat[6] + m1->mat[5] * m2->mat[7] + m1->mat[8] * m2->mat[8];

	memcpy(dst->mat, mat, sizeof(float) * 9);

	return dst;
}

static inline struct mat3 *
m3_mul_scalar(
	struct mat3 *dst,
	const struct mat3 *m,
	const float f)
{
	dst->mat[0] = m->mat[0] * f;
	dst->mat[1] = m->mat[1] * f;
	dst->mat[2] = m->mat[2] * f;
	dst->mat[3] = m->mat[3] * f;
	dst->mat[4] = m->mat[4] * f;
	dst->mat[5] = m->mat[5] * f;
	dst->mat[6] = m->mat[6] * f;
	dst->mat[7] = m->mat[7] * f;
	dst->mat[8] = m->mat[8] * f;

	return dst;
}

/*
 * Sets pOut to the transpose of pIn, returns pOut
 */
static inline struct mat3 *
m3_transpose(
	struct mat3 *dst,
	const struct mat3 *src)
{
	float temp[9];

	temp[0] = src->mat[0];
	temp[1] = src->mat[3];
	temp[2] = src->mat[6];
	
	temp[3] = src->mat[1];
	temp[4] = src->mat[4];
	temp[5] = src->mat[7];

	temp[6] = src->mat[2];
	temp[7] = src->mat[5];
	temp[8] = src->mat[8];

	memcpy(&dst->mat, temp, sizeof(float) * 9);

	return dst;
}

static inline struct mat3 *
m3_adjugate(
	struct mat3 *dst,
	const struct mat3 *src)
{
	dst->mat[0] = src->mat[4] * src->mat[8] - src->mat[5] * src->mat[7];
	dst->mat[1] = src->mat[2] * src->mat[7] - src->mat[1] * src->mat[8];
	dst->mat[2] = src->mat[1] * src->mat[5] - src->mat[2] * src->mat[4];
	dst->mat[3] = src->mat[5] * src->mat[6] - src->mat[3] * src->mat[8];
	dst->mat[4] = src->mat[0] * src->mat[8] - src->mat[2] * src->mat[6];
	dst->mat[5] = src->mat[2] * src->mat[3] - src->mat[0] * src->mat[5];
	dst->mat[6] = src->mat[3] * src->mat[7] - src->mat[4] * src->mat[6];
	dst->mat[7] = src->mat[1] * src->mat[6] - src->mat[0] * src->mat[7];
	dst->mat[8] = src->mat[0] * src->mat[4] - src->mat[1] * src->mat[3];

	return dst;
}

static inline struct mat3 *
m3_inverse(
	struct mat3 *dst,
	const struct mat3 *m)
{
	float d = m3_det(m);
	float d_inv;
	struct mat3 adjugate;

	if (d == 0.f)
		return NULL;

	d_inv = 1.f / d;

	m3_adjugate(&adjugate, m);
	m3_mul_scalar(dst, &adjugate, d_inv);

	return dst;
}

/*
 * Builds an X-axis rotation matrix and stores it in pOut, returns pOut
 */
static inline struct mat3 *
m3_rot_x(
	struct mat3 *dst,
	const float radians)
{
	/*
	 *		|  1  0       0      |
	 *	M = 	|  0  cos(A) -sin(A) |
	 *		|  0  sin(A)  cos(A) |
	 */

	dst->mat[0] = 1.f;
	dst->mat[1] = 0.f;
	dst->mat[2] = 0.f;

	dst->mat[3] = 0.f;
	dst->mat[4] = cosf(radians);
	dst->mat[5] = sinf(radians);

	dst->mat[6] = 0.f;
	dst->mat[7] = -sinf(radians);
	dst->mat[8] = cosf(radians);

	return dst;
}

/*
 * Builds a rotation matrix using the rotation around the Y-axis
 * The result is stored in pOut, pOut is returned.
 */
static inline struct mat3 *
m3_rot_y(
	struct mat3 *dst,
	const float radians)
{
	/*
	 *		|  cos(A)  0   sin(A) |
	 *	M = 	|  0       1   0      |
	 *		| -sin(A)  0   cos(A) |
	 */

	dst->mat[0] = cosf(radians);
	dst->mat[1] = 0.f;
	dst->mat[2] = -sinf(radians);

	dst->mat[3] = 0.f;
	dst->mat[4] = 1.f;
	dst->mat[5] = 0.f;

	dst->mat[6] = sinf(radians);
	dst->mat[7] = 0.f;
	dst->mat[8] = cosf(radians);

	return dst;
}

/*
 * Builds a rotation matrix around the Z-axis. The resulting
 * matrix is stored in pOut. pOut is returned.
 */
static inline struct mat3 *
m3_rot_z(
	struct mat3 *dst,
	const float radians)
{
	/*
	 *		|  cos(A)  -sin(A)   0  |
	 *	M =	|  sin(A)   cos(A)   0  |
	 *		|  0        0        1  |
	 */

	dst->mat[0] = cosf(radians);
	dst->mat[1] = -sinf(radians);
	dst->mat[2] = 0.f;

	dst->mat[3] = sinf(radians);
	dst->mat[4] = cosf(radians);
	dst->mat[5] = 0.f;

	dst->mat[6] = 0.f;
	dst->mat[7] = 0.f;
	dst->mat[8] = 1.f;

	return dst;
}

static inline struct mat3 *
m3_rot_quat(
	struct mat3 *dst,
	const struct quat *src)
{
	/* First row */
	dst->mat[0] = 1.f - 2.f * (src->y * src->y + src->z * src->z);
	dst->mat[1] = 2.f * (src->x * src->y - src->w * src->z);
	dst->mat[2] = 2.f * (src->x * src->z + src->w * src->y);

	/* Second row */
	dst->mat[3] = 2.f * (src->x * src->y + src->w * src->z);
	dst->mat[4] = 1.f - 2.f * (src->x * src->x + src->z * src->z);
	dst->mat[5] = 2.f * (src->y * src->z - src->w * src->x);

	/* Third row */
	dst->mat[6] = 2.f * (src->x * src->z - src->w * src->y);
	dst->mat[7] = 2.f * (src->y * src->z + src->w * src->x);
	dst->mat[8] = 1.f - 2.f * (src->x * src->x + src->y * src->y);

	return dst;
}

static inline struct mat3 *
m3_rot_axis_angle(
	struct mat3 *dst,
	const struct vec3 *axis,
	const float radians)
{
	float rcos = cosf(radians);
	float rsin = sinf(radians);

	dst->mat[0] = rcos + axis->x * axis->x * (1 - rcos);
	dst->mat[1] = axis->z * rsin + axis->y * axis->x * (1 - rcos);
	dst->mat[2] = -axis->y * rsin + axis->z * axis->x * (1 - rcos);

	dst->mat[3] = -axis->z * rsin + axis->x * axis->y * (1 - rcos);
	dst->mat[4] = rcos + axis->y * axis->y * (1 - rcos);
	dst->mat[5] = axis->x * rsin + axis->z * axis->y * (1 - rcos);

	dst->mat[6] = axis->y * rsin + axis->x * axis->z * (1 - rcos);
	dst->mat[7] = -axis->x * rsin + axis->y * axis->z * (1 - rcos);
	dst->mat[8] = rcos + axis->z * axis->z * (1 - rcos);

	return dst;
}

/*
 * Builds a rotation matrix from pitch, yaw and roll. The resulting
 * matrix is stored in pOut and pOut is returned
 */
static inline struct mat3 *
m3_rot_pitch_yaw_roll(
	struct mat3 *dst,
	const float pitch,
	const float yaw,
	const float roll)
{
	struct mat3 yaw_matrix;
	struct mat3 roll_matrix;
	struct mat3 pitch_matrix;

	m3_rot_y(&yaw_matrix, yaw);

	m3_rot_x(&pitch_matrix, pitch);

	m3_rot_z(&roll_matrix, roll);

	m3_mul(dst, &pitch_matrix, &roll_matrix);
	m3_mul(dst, &yaw_matrix, dst);

	return dst;
}


static inline struct mat3 *
m3_look_at(
	struct mat3 *dst,
	const struct vec3 *eye,
	const struct vec3 *center,
	const struct vec3 *up)
{
	struct vec3 f, n_up, s, u;

	v3_sub(&f, center, eye);
	v3_norm(&f, &f);

	v3_copy(&n_up, up);
	v3_norm(&n_up, &n_up);

	v3_cross(&s, &f, &n_up);
	v3_norm(&s, &s);

	v3_cross(&u, &s, &f);
	v3_norm(&s, &s);

	dst->mat[0] = s.x;
	dst->mat[3] = s.y;
	dst->mat[6] = s.z;

	dst->mat[1] = u.x;
	dst->mat[4] = u.y;
	dst->mat[7] = u.z;

	dst->mat[2] = -f.x;
	dst->mat[5] = -f.y;
	dst->mat[8] = -f.z;

	return dst;
}

/*
 * Builds a scaling matrix
 */
static inline struct mat3 *
m3_scale(
	struct mat3 *dst,
	const float x,
	const float y)
{
	m3_ident(dst);
	
	dst->mat[0] = x;
	dst->mat[4] = y;

	return dst;
}

static inline struct mat3 *
m3_translate(
	struct mat3 *dst,
	const float x,
	const float y)
{
	m3_ident(dst);

	dst->mat[6] = x;
	dst->mat[7] = y;

	return dst;
}

static inline struct vec3 *
m3_up(
	const struct mat3 *m,
	struct vec3 *v)
{
	v->x = m->mat[3];
	v->y = m->mat[4];
	v->z = m->mat[5];

	return v3_norm(v, v);
}

static inline struct vec3 *
m3_fwd(
	const struct mat3 *m,
	struct vec3 *v)
{
	v->x = m->mat[6];
	v->y = m->mat[7];
	v->z = m->mat[8];

	return v3_norm(v, v);
}

static inline struct vec3 *
m3_right(
	const struct mat3 *m,
	struct vec3 *v)
{
	v->x = m->mat[0];
	v->y = m->mat[1];
	v->z = m->mat[2];

	return v3_norm(v, v);
}

#endif /* _NE_MATH_MAT3_H_ */

