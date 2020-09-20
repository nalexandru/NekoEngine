/* NekoEngine
 *
 * mat4.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions
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

#ifndef _NE_MATH_ALTIVEC_MAT4_H_
#define _NE_MATH_ALTIVEC_MAT4_H_

#include <Math/defs.h>

#ifdef USE_ALTIVEC

#include <Math/vec3.h>
#include <Math/quat.h>

// fu m$
#undef near
#undef far

/*
A 4x4 matrix

      | 0   4   8  12 |
mat = | 1   5   9  13 |
      | 2   6  10  14 |
      | 3   7  11  15 |
*/

/*
 * Fills a mat4 structure with the values from a 16
 * element array of floats
 * @Params pOut - A pointer to the destination matrix
 * 		   pMat - A 16 element array of floats
 * @Return Returns pOut so that the call can be nested
 */
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

/*
 * Assigns the value of pIn to pOut
 */
static inline struct mat4 *
m4_copy(struct mat4 *dst,
	const struct mat4 *src)
{
	memcpy(dst->m, src->m, sizeof(float) * 16);
	return dst;
}

/*
 * Sets pOut to an identity matrix returns pOut
 * @Params pOut - A pointer to the matrix to set to identity
 * @Return Returns pOut so that the call can be nested
 */
static inline struct mat4 *
m4_ident(struct mat4 *m)
{
	memset(m->m, 0, sizeof(float) * 16);
	m->m[0] = m->m[5] = m->m[10] = m->m[15] = 1.f;
	return m;
}

static inline struct mat4 *
m4_init_m3(struct mat4 *pOut, const struct mat3 *pIn)
{
	m4_ident(pOut);

	pOut->m[0] = pIn->mat[0];
	pOut->m[1] = pIn->mat[1];
	pOut->m[2] = pIn->mat[2];
	pOut->m[3] = 0.0;

	pOut->m[4] = pIn->mat[3];
	pOut->m[5] = pIn->mat[4];
	pOut->m[6] = pIn->mat[5];
	pOut->m[7] = 0.0;

	pOut->m[8] = pIn->mat[6];
	pOut->m[9] = pIn->mat[7];
	pOut->m[10] = pIn->mat[8];
	pOut->m[11] = 0.0;

	return pOut;
}

/*
 * Multiplies pM1 with pM2, stores the result in pOut, returns pOut
 */
static inline struct mat4 *
m4_mul(struct mat4 *dst, const struct mat4 *m1, const struct mat4 *m2)
{
#if defined(SYS_ARCH_X86_64) || defined(SYS_ARCH_X86)
	__m128 tmp0 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0x00], 0x00), m1->sm[0]);
	tmp0 = _mm_add_ps(tmp0, _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0], 0x55), m1->sm[1]));
	tmp0 = _mm_add_ps(tmp0, _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0], 0xAA), m1->sm[2]));
	tmp0 = _mm_add_ps(tmp0, _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0], 0xFF), m1->sm[3]));

	__m128 tmp1 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0x00), m1->sm[0]);
	tmp1 = _mm_add_ps(tmp1, _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0x55), m1->sm[1]));
	tmp1 = _mm_add_ps(tmp1, _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0xAA), m1->sm[2]));
	tmp1 = _mm_add_ps(tmp1, _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0xFF), m1->sm[3]));

	__m128 tmp2 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0x00), m1->sm[0]);
	tmp2 = _mm_add_ps(tmp2, _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0x55), m1->sm[1]));
	tmp2 = _mm_add_ps(tmp2, _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0xAA), m1->sm[2]));
	tmp2 = _mm_add_ps(tmp2, _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0xFF), m1->sm[3]));

	__m128 tmp3 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0x00), m1->sm[0]);
	tmp3 = _mm_add_ps(tmp3, _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0x55), m1->sm[1]));
	tmp3 = _mm_add_ps(tmp3, _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0xAA), m1->sm[2]));
	tmp3 = _mm_add_ps(tmp3, _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0xFF), m1->sm[3]));

	dst->sm[0] = tmp0;
	dst->sm[1] = tmp1;
	dst->sm[2] = tmp2;
	dst->sm[3] = tmp3;
#else
	float mat[16];

	mat[0] = m1->m[0] * m2->m[0] + m1->m[4] * m2->m[1] + m1->m[8]  * m2->m[2]  + m1->m[12] * m2->m[3];
	mat[1] = m1->m[1] * m2->m[0] + m1->m[5] * m2->m[1] + m1->m[9]  * m2->m[2]  + m1->m[13] * m2->m[3];
	mat[2] = m1->m[2] * m2->m[0] + m1->m[6] * m2->m[1] + m1->m[10] * m2->m[2]  + m1->m[14] * m2->m[3];
	mat[3] = m1->m[3] * m2->m[0] + m1->m[7] * m2->m[1] + m1->m[11] * m2->m[2]  + m1->m[15] * m2->m[3];

	mat[4] = m1->m[0] * m2->m[4] + m1->m[4] * m2->m[5] + m1->m[8]  * m2->m[6]  + m1->m[12] * m2->m[7];
	mat[5] = m1->m[1] * m2->m[4] + m1->m[5] * m2->m[5] + m1->m[9]  * m2->m[6]  + m1->m[13] * m2->m[7];
	mat[6] = m1->m[2] * m2->m[4] + m1->m[6] * m2->m[5] + m1->m[10] * m2->m[6]  + m1->m[14] * m2->m[7];
	mat[7] = m1->m[3] * m2->m[4] + m1->m[7] * m2->m[5] + m1->m[11] * m2->m[6]  + m1->m[15] * m2->m[7];

	mat[8] = m1->m[0] * m2->m[8] + m1->m[4] * m2->m[9] + m1->m[8]  * m2->m[10] + m1->m[12] * m2->m[11];
	mat[9] = m1->m[1] * m2->m[8] + m1->m[5] * m2->m[9] + m1->m[9]  * m2->m[10] + m1->m[13] * m2->m[11];
	mat[10] = m1->m[2] * m2->m[8]  + m1->m[6] * m2->m[9]  + m1->m[10] * m2->m[10] + m1->m[14] * m2->m[11];
	mat[11] = m1->m[3] * m2->m[8]  + m1->m[7] * m2->m[9]  + m1->m[11] * m2->m[10] + m1->m[15] * m2->m[11];

	mat[12] = m1->m[0] * m2->m[12] + m1->m[4] * m2->m[13] + m1->m[8]  * m2->m[14] + m1->m[12] * m2->m[15];
	mat[13] = m1->m[1] * m2->m[12] + m1->m[5] * m2->m[13] + m1->m[9]  * m2->m[14] + m1->m[13] * m2->m[15];
	mat[14] = m1->m[2] * m2->m[12] + m1->m[6] * m2->m[13] + m1->m[10] * m2->m[14] + m1->m[14] * m2->m[15];
	mat[15] = m1->m[3] * m2->m[12] + m1->m[7] * m2->m[13] + m1->m[11] * m2->m[14] + m1->m[15] * m2->m[15];

	memcpy(dst->m, mat, sizeof(float) * 16);
#endif

	return dst;
}

static inline struct mat4 *
m4_mul_scalar(struct mat4 *dst, const struct mat4 *m, const float f)
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
	dst->m[9] = m->m[8] * f;
	dst->m[10] = m->m[9] * f;
	dst->m[11] = m->m[11] * f;

	dst->m[12] = m->m[12] * f;
	dst->m[13] = m->m[13] * f;
	dst->m[14] = m->m[14] * f;
	dst->m[15] = m->m[15] * f;

	return dst;
}

/*
 * Sets pOut to the transpose of pIn, returns pOut
 */
static inline struct mat4 *
m4_transpose(struct mat4 *dst, const struct mat4 *src)
{
	int x, z;

	for (z = 0; z < 4; ++z)
		for (x = 0; x < 4; ++x)
			dst->m[(z * 4) + x] = src->m[(x * 4) + z];

	return dst;
}

/*
 * Calculates the inverse of pM and stores the result in
 * pOut.
 * @Return Returns NULL if there is no inverse, else pOut
 */
static inline struct mat4 *
m4_inverse(struct mat4 *dst, const struct mat4 *src)
{
	struct mat4 tmp;
	float det;
	int i;

	tmp.m[0] = src->m[5]  * src->m[10] * src->m[15] -
		src->m[5]  * src->m[11] * src->m[14] -
		src->m[9]  * src->m[6]  * src->m[15] +
		src->m[9]  * src->m[7]  * src->m[14] +
		src->m[13] * src->m[6]  * src->m[11] -
		src->m[13] * src->m[7]  * src->m[10];

	tmp.m[4] = -src->m[4]  * src->m[10] * src->m[15] +
		src->m[4]  * src->m[11] * src->m[14] +
		src->m[8]  * src->m[6]  * src->m[15] -
		src->m[8]  * src->m[7]  * src->m[14] -
		src->m[12] * src->m[6]  * src->m[11] +
		src->m[12] * src->m[7]  * src->m[10];

	tmp.m[8] = src->m[4]  * src->m[9] * src->m[15] -
		src->m[4]  * src->m[11] * src->m[13] -
		src->m[8]  * src->m[5] * src->m[15] +
		src->m[8]  * src->m[7] * src->m[13] +
		src->m[12] * src->m[5] * src->m[11] -
		src->m[12] * src->m[7] * src->m[9];

	tmp.m[12] = -src->m[4]  * src->m[9] * src->m[14] +
		src->m[4]  * src->m[10] * src->m[13] +
		src->m[8]  * src->m[5] * src->m[14] -
		src->m[8]  * src->m[6] * src->m[13] -
		src->m[12] * src->m[5] * src->m[10] +
		src->m[12] * src->m[6] * src->m[9];

	tmp.m[1] = -src->m[1]  * src->m[10] * src->m[15] +
		src->m[1]  * src->m[11] * src->m[14] +
		src->m[9]  * src->m[2] * src->m[15] -
		src->m[9]  * src->m[3] * src->m[14] -
		src->m[13] * src->m[2] * src->m[11] +
		src->m[13] * src->m[3] * src->m[10];

	tmp.m[5] = src->m[0]  * src->m[10] * src->m[15] -
		src->m[0]  * src->m[11] * src->m[14] -
		src->m[8]  * src->m[2] * src->m[15] +
		src->m[8]  * src->m[3] * src->m[14] +
		src->m[12] * src->m[2] * src->m[11] -
		src->m[12] * src->m[3] * src->m[10];

	tmp.m[9] = -src->m[0]  * src->m[9] * src->m[15] +
		src->m[0]  * src->m[11] * src->m[13] +
		src->m[8]  * src->m[1] * src->m[15] -
		src->m[8]  * src->m[3] * src->m[13] -
		src->m[12] * src->m[1] * src->m[11] +
		src->m[12] * src->m[3] * src->m[9];

	tmp.m[13] = src->m[0]  * src->m[9] * src->m[14] -
		src->m[0]  * src->m[10] * src->m[13] -
		src->m[8]  * src->m[1] * src->m[14] +
		src->m[8]  * src->m[2] * src->m[13] +
		src->m[12] * src->m[1] * src->m[10] -
		src->m[12] * src->m[2] * src->m[9];

	tmp.m[2] = src->m[1]  * src->m[6] * src->m[15] -
		src->m[1]  * src->m[7] * src->m[14] -
		src->m[5]  * src->m[2] * src->m[15] +
		src->m[5]  * src->m[3] * src->m[14] +
		src->m[13] * src->m[2] * src->m[7] -
		src->m[13] * src->m[3] * src->m[6];

	tmp.m[6] = -src->m[0]  * src->m[6] * src->m[15] +
		src->m[0]  * src->m[7] * src->m[14] +
		src->m[4]  * src->m[2] * src->m[15] -
		src->m[4]  * src->m[3] * src->m[14] -
		src->m[12] * src->m[2] * src->m[7] +
		src->m[12] * src->m[3] * src->m[6];

	tmp.m[10] = src->m[0]  * src->m[5] * src->m[15] -
		src->m[0]  * src->m[7] * src->m[13] -
		src->m[4]  * src->m[1] * src->m[15] +
		src->m[4]  * src->m[3] * src->m[13] +
		src->m[12] * src->m[1] * src->m[7] -
		src->m[12] * src->m[3] * src->m[5];

	tmp.m[14] = -src->m[0]  * src->m[5] * src->m[14] +
		src->m[0]  * src->m[6] * src->m[13] +
		src->m[4]  * src->m[1] * src->m[14] -
		src->m[4]  * src->m[2] * src->m[13] -
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

/*
 * Builds an X-axis rotation matrix and stores it in pOut, returns pOut
 */
static inline struct mat4 *
m4_rot_x(struct mat4 *dst, const float radians)
{
	/*
	 *		|  1  0       0       0 |
	 *	M =	|  0  cos(A) -sin(A)  0 |
	 *		|  0  sin(A)  cos(A)  0 |
	 *		|  0  0       0       1 |
	 */

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

/*
 * Builds a rotation matrix using the rotation around the Y-axis
 * The result is stored in pOut, pOut is returned.
 */
static inline struct mat4 *
m4_rot_y(struct mat4 *dst, const float radians)
{
	/*
	 *		|  cos(A)  0   sin(A)  0 |
	 *	M =	|  0       1   0       0 |
	 *		| -sin(A)  0   cos(A)  0 |
	 *		|  0       0   0       1 |
	 */

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

/*
 * Builds a rotation matrix around the Z-axis. The resulting
 * matrix is stored in pOut. pOut is returned.
 */
static inline struct mat4 *
m4_rot_z(struct mat4 *dst, const float radians)
{
	/*
	 *		|  cos(A)  -sin(A)   0   0 |
	 *	M =	|  sin(A)   cos(A)   0   0 |
	 *		|  0        0        1   0 |
	 *		|  0        0        0   1 |
	 */

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

/*
 * Converts a quaternion to a rotation matrix,
 * the result is stored in pOut, returns pOut
 */
static inline struct mat4 *
m4_rot_quat(struct mat4 *dst, const struct quat *pQ)
{
	float xx = pQ->x * pQ->x;
	float xy = pQ->x * pQ->y;
	float xz = pQ->x * pQ->z;
	float xw = pQ->x * pQ->w;
	
	float yy = pQ->y * pQ->y;
	float yz = pQ->y * pQ->z;
	float yw = pQ->y * pQ->w;
	
	float zz = pQ->z * pQ->z;
	float zw = pQ->z * pQ->w;

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

/*
 * Build a rotation matrix from an axis and an angle. Result is stored in pOut.
 * pOut is returned.
 */
static inline struct mat4 *
m4_rot_axis_angle(struct mat4 *dst, const struct vec3 *axis, float deg)
{
	struct quat quat;
	quat_rot_axis_angle(&quat, axis, deg);
	m4_rot_quat(dst, &quat);
	return dst;
}

/*
 * Builds a rotation matrix from pitch, yaw and roll. The resulting
 * matrix is stored in pOut and pOut is returned
 */
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

/*
 * Builds a translation matrix in the same way as gluLookAt()
 * the resulting matrix is stored in pOut. pOut is returned.
 */
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

/*
 * Builds a scaling matrix
 */
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

/*
 * Builds a translation matrix. All other elements in the matrix
 * will be set to zero except for the diagonal which is set to 1.0
 */
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

/*
 * Get the up vector from a matrix. pIn is the matrix you
 * wish to extract the vector from. pOut is a pointer to the
 * vec3 structure that should hold the resulting vector
 */
static inline struct vec3 *
m4_up(struct vec3 *v, const struct mat4 *m)
{
	v3_mul_m4(v, &KM_VEC3_POS_Y, m);
	return v3_norm(v, v);
}

/*
 * Extract the forward vector from a 4x4 matrix. The result is
 * stored in pOut. Returns pOut.
 */
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

/*
 * Extract the right vector from a 4x4 matrix. The result is
 * stored in pOut. Returns pOut.
 */
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
	dst->r[3][2] = -(far * near) / (far - near);
	dst->r[2][3] = -1.0f;
	dst->r[3][2] = -(2.f * far * near) / (far - near);

	return dst;
}

static inline struct mat4 *
m4_infinite_perspective_rz(struct mat4 *dst, float fov_y, float aspect, float near)
{
	memset(dst, 0x0, sizeof(*dst));

	const float f = 1.f / tanf(deg_to_rad(fov_y) / 2.f);

	dst->m[0] = f / aspect;
	dst->m[5] = f;
	dst->m[11] = -1.f;
	dst->m[14] = near;

	return dst;
}

/*
 * Creates an orthographic projection matrix like glOrtho
 */
static inline struct mat4 *
m4_ortho(struct mat4 *dst,
	float left, float right, float bottom, float top,
	float near, float far)
{
	float tx = -((right + left) / (right - left));
	float ty = -((top + bottom) / (top - bottom));
	float tz = -((far + near) / (far - near));

	m4_ident(dst);
	dst->m[0] = 2.f / (right - left);
	dst->m[5] = 2.f / (top - bottom);
	dst->m[10] = -2.f / (far - near);
	dst->m[12] = tx;
	dst->m[13] = ty;
	dst->m[14] = tz;

	return dst;
}

#endif

#endif /* _NE_MATH_ALTIVEC_MAT4_H_ */

