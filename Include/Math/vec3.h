/* NekoEngine
 *
 * vec3.h
 * Author: Alexandru Naiman
 *
 * 3 component vector functions
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

#ifndef _NE_MATH_VEC3_H_
#define _NE_MATH_VEC3_H_

#include <Math/defs.h>
#include <Math/func.h>

/*
 * Fill a vec3 structure using 3 floating point values
 * The result is store in pOut, returns pOut
 */
static inline struct vec3 *
v3(struct vec3 *v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;

	return v;
}

static inline struct vec3 *
v3_fill(struct vec3 *v, float f)
{
	v->x = v->y = v->z = f;

	return v;
}

/*
 * Assigns pIn to pOut. Returns pOut. If pIn and pOut are the same
 * then nothing happens but pOut is still returned
 */
static inline struct vec3 *
v3_copy(struct vec3 *dst, const struct vec3 *src)
{
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;

	return dst;
}

/*
 * Sets all the elements of pOut to zero. Returns pOut.
 */
static inline struct vec3 *
v3_zero(struct vec3 *dst)
{
	dst->x = 0.f;
	dst->y = 0.f;
	dst->z = 0.f;

	return dst;
}

/*
 * Returns the square of the length of the vector
 */
static inline float
v3_len_sq(const struct vec3 *v)
{
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

/*
 * Returns the length of the vector
 */
static inline float
v3_len(const struct vec3 *v)
{
	return sqrtf((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

/*
 * Returns the vector passed in set to unit length
 * the result is stored in pOut.
 */
static inline struct vec3 *
v3_norm(struct vec3 *dst, const struct vec3 *src)
{
	float l;

	if (!src->x && !src->y && !src->z)
		return v3_copy(dst, src);

	l = 1.f / v3_len(src);

	dst->x = src->x * l;
	dst->y = src->y * l;
	dst->z = src->z * l;

	return dst;
}

/*
 * Returns the interpolation of 2 4D vectors based on t.
 */
static inline struct vec3 *
v3_lerp(struct vec3 *dst, const struct vec3 *v1, const struct vec3 *v2, float t)
{
	dst->x = v1->x + t * (v2->x - v1->x);
	dst->y = v1->y + t * (v2->y - v1->y);
	dst->z = v1->z + t * (v2->z - v1->z);

	return dst;
}

/*
 * Adds 2 vectors and returns the result. The resulting
 * vector is stored in pOut.
 */
static inline struct vec3 *
v3_add(struct vec3 *dst, const struct vec3 *v1, const struct vec3 *v2)
{
	dst->x = v1->x + v2->x;
	dst->y = v1->y + v2->y;
	dst->z = v1->z + v2->z;

	return dst;
}

/*
 * Subtracts 2 vectors and returns the result. The result is stored in
 * pOut.
 */
static inline struct vec3 *
v3_sub(struct vec3 *dst, const struct vec3 *v1, const struct vec3 *v2)
{
	dst->x = v1->x - v2->x;
	dst->y = v1->y - v2->y;
	dst->z = v1->z - v2->z;

	return dst;
}

static inline struct vec3 *
v3_mul(struct vec3 *dst, const struct vec3 *v1, const struct vec3 *v2)
{
	dst->x = v1->x * v2->x;
	dst->y = v1->y * v2->y;
	dst->z = v1->z * v2->z;

	return dst;
}

static inline struct vec3 *
v3_div(struct vec3 *dst, const struct vec3 *v1, const struct vec3 *v2)
{
	dst->x = v1->x / v2->x;
	dst->y = v1->y / v2->y;
	dst->z = v1->z / v2->z;

	return dst;
}

/*
 * Adds 2 vectors and returns the result. The resulting
 * vector is stored in pOut.
 */
static inline struct vec3 *
v3_adds(struct vec3 *dst, const struct vec3 *v, const float s)
{
	dst->x = v->x + s;
	dst->y = v->y + s;
	dst->z = v->z + s;

	return dst;
}

/*
 * Subtracts 2 vectors and returns the result. The result is stored in
 * pOut.
 */
static inline struct vec3 *
v3_subs(struct vec3 *dst, const struct vec3 *v, const float s)
{
	dst->x = v->x - s;
	dst->y = v->y - s;
	dst->z = v->z - s;

	return dst;
}

static inline struct vec3 *
v3_muls(struct vec3 *dst, const struct vec3 *v, const float s)
{
	dst->x = v->x * s;
	dst->y = v->y * s;
	dst->z = v->z * s;

	return dst;
}

static inline struct vec3 *
v3_divs(struct vec3 *dst, const struct vec3 *v, const float s)
{
	dst->x = v->x / s;
	dst->y = v->y / s;
	dst->z = v->z / s;

	return dst;
}

/*
 * Returns the cosine of the angle between 2 vectors
 */
static inline float
v3_dot(const struct vec3 *v1, const struct vec3 *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

/*
 * Returns a vector perpendicular to 2 other vectors.
 * The result is stored in pOut.
 */
static inline struct vec3 *
v3_cross(struct vec3 *dst, const struct vec3 *v1, const struct vec3 *v2)
{
	dst->x = (v1->y * v2->z) - (v1->z * v2->y);
	dst->y = (v1->z * v2->x) - (v1->x * v2->z);
	dst->z = (v1->x * v2->y) - (v1->y * v2->x);

	return dst;
}

/*
 * Scales a vector to length s. Does not normalize first,
 * you should do that!
 */
static inline struct vec3 *
v3_scale(struct vec3 *dst, const struct vec3 *src, const float s)
{
	v3_norm(dst, src);
	return v3_muls(dst, dst, s);
}

/*
 * Get the rotations that would make a (0,0,1) direction vector point
 * in the same direction as this direction vector.  Useful for
 * orienting vector towards a point.
 *
 * Returns a rotation vector containing the X (pitch) and Y (raw)
 * rotations (in degrees) that when applied to a +Z (e.g. 0, 0, 1)
 * direction vector would make it point in the same direction as this
 * vector. The Z (roll) rotation is always 0, since two Euler
 * rotations are sufficient to point in any given direction.
 *
 * Code ported from Irrlicht: http://irrlicht.sourceforge.net/
 */
static inline struct vec3 *
v3_angle(struct vec3 *dst, const struct vec3 *src)
{
	const float z1 = sqrtf(src->x * src->x + src->z * src->z);

	dst->y = rad_to_deg(atan2f(src->x, src->z));
	if (dst->y < 0.f)
		dst->y += 360.f;
	if (dst->y >= 360.f)
		dst->y -= 360.f;

	dst->x = rad_to_deg(atan2f(z1, src->y)) - 90.f;
	if (dst->x < 0.f)
		dst->x += 360.f;
	if (dst->x >= 360.f)
		dst->x -= 360.f;

	return dst;
}

static inline float
v3_distance(const struct vec3 *v1, const struct vec3 *v2)
{
	struct vec3 diff;

	v3_sub(&diff, v2, v1);

	return fabsf(v3_len(&diff));
}

/*
 * Builds a direction vector from input vector.
 *
 * Input vector is assumed to be rotation vector composed from 3 Euler
 * angle rotations, in degrees.  The forwards vector will be rotated
 * by the input vector
 *
 * Code ported from Irrlicht: http://irrlicht.sourceforge.net/
 */
static inline struct vec3 *
v3_dir(struct vec3 *dst, const struct vec3 *src, const struct vec3 *fwd)
{
	const float xr = deg_to_rad(src->x);
	const float yr = deg_to_rad(src->y);
	const float zr = deg_to_rad(src->z);

	const float cr = cosf(xr);
	const float sr = sinf(xr);

	const float cp = cosf(yr);
	const float sp = sinf(yr);

	const float cy = cosf(zr);
	const float sy = sinf(zr);

	const float srsp = sr*sp;
	const float crsp = cr*sp;

	const float mat[] = {
		(cp * cy), (cp * sy), (-sp),
		(srsp * cy - cr * sy), (srsp * sy + cr * cy), (sr * cp),
		(crsp * cy + sr * sy), (crsp * sy - sr * cy), (cr * cp)
	};

	dst->x = fwd->x * mat[0] +
		fwd->y * mat[3] +
		fwd->z * mat[6];

	dst->y = fwd->x * mat[1] +
		fwd->y * mat[4] +
		fwd->z * mat[7];

	dst->z = fwd->x * mat[2] +
		fwd->y * mat[5] +
		fwd->z * mat[8];

	return dst;
}

static inline struct vec3 *
v3_project_plane(struct vec3 *d, const struct vec3 *pt, const struct plane *pl)
{
	struct vec3 n;
	float distance;

	v3(&n, pl->a, pl->b, pl->c);
	v3_norm(&n, &n);

	distance = -v3_dot(&n, pt);
	v3_scale(&n, &n, distance);
	v3_add(d, pt, &n);

	return d;
}

static inline struct vec3 *
v3_project_v3(const struct vec3 *w, const struct vec3 *v, struct vec3 *p)
{
	struct vec3 u_w, u_v;
	float cos0;

	v3_norm(&u_w, w);
	v3_norm(&u_v, v);

	cos0 = v3_dot(&u_w, &u_v);

	v3_scale(p, &u_w, v3_len(w) * cos0);

	return p;
}

/*
 * Reflects a vector about a given surface normal. The surface
 * normal is assumed to be of unit length.
 */
static inline struct vec3 *
v3_reflect(struct vec3 *dst, const struct vec3 *src, const struct vec3 *normal)
{
	struct vec3 tmp;
	return v3_sub(dst, src,
		v3_scale(&tmp, normal, 2.f * v3_dot(src, normal)));
}

/*
 * swaps the values in one vector with another
 * NB does not return a value unlike normal
 */
static inline void
v3_swap(struct vec3 *a, struct vec3 *b)
{
	float x, y,z;
	x = a->x;
	a->x = b->x;
	b->x = x;

	y = a->y;
	a->y = b->y;
	b->y = y;

	z = a->z;
	a->z = b->z;
	b->z = z;
}

/*
 * Returns true if the 2 vectors are approximately equal
 */
static inline bool
v3_equal(const struct vec3 *a, const struct vec3 *b)
{
	return float_equal(a->x, b->x) &&
		float_equal(a->y, b->y) &&
		float_equal(a->z, b->z);
}

static inline struct vec3 *
v3_mul_m3(struct vec3 *dst, const struct vec3 *v, const struct mat3 *m)
{
	dst->x = v->x * m->mat[0] + v->y * m->mat[3] + v->z * m->mat[6];
	dst->y = v->x * m->mat[1] + v->y * m->mat[4] + v->z * m->mat[7];
	dst->z = v->x * m->mat[2] + v->y * m->mat[5] + v->z * m->mat[8];

	return dst;
}

/*
 * Multiplies vector (x, y, z, 1) by a given matrix. The result
 * is stored in pOut. pOut is returned.
 */
static inline struct vec3 *
v3_mul_m4(struct vec3 *dst, const struct vec3 *v, const struct mat4 *m)
{
	dst->x = v->x * m->m[0] + v->y * m->m[4] + v->z * m->m[8] + m->m[12];
	dst->y = v->x * m->m[1] + v->y * m->m[5] + v->z * m->m[9] + m->m[13];
	dst->z = v->x * m->m[2] + v->y * m->m[6] + v->z * m->m[10] + m->m[14];

	return dst;
}

static const struct vec3 KM_VEC3_POS_Z = { 0, 0, 1 };
static const struct vec3 KM_VEC3_NEG_Z = { 0, 0, -1 };
static const struct vec3 KM_VEC3_POS_Y = { 0, 1, 0 };
static const struct vec3 KM_VEC3_NEG_Y = { 0, -1, 0 };
static const struct vec3 KM_VEC3_NEG_X = { -1, 0, 0 };
static const struct vec3 KM_VEC3_POS_X = { 1, 0, 0 };
static const struct vec3 KM_VEC3_ZERO = { 0, 0, 0 };

#endif /* _NE_MATH_VEC3_H_ */

