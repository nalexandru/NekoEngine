#ifndef _NE_MATH_VEC3_H_
#define _NE_MATH_VEC3_H_

#include <Math/defs.h>
#include <Math/func.h>

/*
 * Fill a NeVec3 structure using 3 floating point values
 * The result is store in pOut, returns pOut
 */
static inline struct NeVec3 *
M_Vec3(struct NeVec3 *v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;

	return v;
}

static inline struct NeVec3 *
M_FillVec3(struct NeVec3 *v, float f)
{
	v->x = v->y = v->z = f;

	return v;
}

/*
 * Assigns pIn to pOut. Returns pOut. If pIn and pOut are the same
 * then nothing happens but pOut is still returned
 */
static inline struct NeVec3 *
M_CopyVec3(struct NeVec3 *dst, const struct NeVec3 *src)
{
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;

	return dst;
}

/*
 * Sets all the elements of pOut to zero. Returns pOut.
 */
static inline struct NeVec3 *
M_ZeroVec3(struct NeVec3 *dst)
{
	dst->x = 0.f;
	dst->y = 0.f;
	dst->z = 0.f;

	return dst;
}

/*
 * Adds 2 vectors and returns the result. The resulting
 * vector is stored in pOut.
 */
static inline struct NeVec3 *
M_AddVec3(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	dst->x = v1->x + v2->x;
	dst->y = v1->y + v2->y;
	dst->z = v1->z + v2->z;

	return dst;
}

static inline struct NeVec3 *
M_AddVec3S(struct NeVec3 *dst, const struct NeVec3 *v1, const float s)
{
	dst->x = v1->x + s;
	dst->y = v1->y + s;
	dst->z = v1->z + s;

	return dst;
}

/*
 * Subtracts 2 vectors and returns the result. The result is stored in
 * pOut.
 */
static inline struct NeVec3 *
M_SubVec3(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	dst->x = v1->x - v2->x;
	dst->y = v1->y - v2->y;
	dst->z = v1->z - v2->z;

	return dst;
}

static inline struct NeVec3 *
M_SubVec3S(struct NeVec3 *dst, const struct NeVec3 *v1, const float s)
{
	dst->x = v1->x - s;
	dst->y = v1->y - s;
	dst->z = v1->z - s;

	return dst;
}

static inline struct NeVec3 *
M_MulVec3(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	dst->x = v1->x * v2->x;
	dst->y = v1->y * v2->y;
	dst->z = v1->z * v2->z;

	return dst;
}

static inline struct NeVec3 *
M_MulVec3S(struct NeVec3 *dst, const struct NeVec3 *v1, const float s)
{
	dst->x = v1->x * s;
	dst->y = v1->y * s;
	dst->z = v1->z * s;

	return dst;
}

static inline struct NeVec3 *
M_DivVec3(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	dst->x = v1->x / v2->x;
	dst->y = v1->y / v2->y;
	dst->z = v1->z / v2->z;

	return dst;
}

static inline struct NeVec3 *
M_DivVec3S(struct NeVec3 *dst, const struct NeVec3 *v, const float s)
{
	dst->x = v->x / s;
	dst->y = v->y / s;
	dst->z = v->z / s;

	return dst;
}

/*
 * Returns the square of the length of the vector
 */
static inline float
M_Vec3LengthSquared(const struct NeVec3 *v)
{
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

/*
 * Returns the length of the vector
 */
static inline float
M_Vec3Length(const struct NeVec3 *v)
{
	return sqrtf((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

/*
 * Returns the vector passed in set to unit length
 * the result is stored in pOut.
 */
static inline struct NeVec3 *
M_NormalizeVec3(struct NeVec3 *dst, const struct NeVec3 *src)
{
	float l;

	if (!src->x && !src->y && !src->z)
		return M_CopyVec3(dst, src);

	l = 1.f / M_Vec3Length(src);

	dst->x = src->x * l;
	dst->y = src->y * l;
	dst->z = src->z * l;

	return dst;
}

/*
 * Returns the interpolation of 2 4D vectors based on t.
 */
static inline struct NeVec3 *
M_LerpVec3(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2, float t)
{
	dst->x = v1->x + t * (v2->x - v1->x);
	dst->y = v1->y + t * (v2->y - v1->y);
	dst->z = v1->z + t * (v2->z - v1->z);

	return dst;
}

/*
 * Returns the cosine of the angle between 2 vectors
 */
static inline float
M_DotVec3(const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

/*
 * Returns a vector perpendicular to 2 other vectors.
 * The result is stored in pOut.
 */
static inline struct NeVec3 *
M_CrossVec3(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
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
static inline struct NeVec3 *
M_ScaleVec3(struct NeVec3 *dst, const struct NeVec3 *src, const float s)
{
	return M_MulVec3S(dst, M_NormalizeVec3(dst, src), s);
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
static inline struct NeVec3 *
M_Vec3Angle(struct NeVec3 *dst, const struct NeVec3 *src)
{
	const float z1 = sqrtf(src->x * src->x + src->z * src->z);

	dst->y = M_RadToDeg(atan2f(src->x, src->z));
	if (dst->y < 0.f)
		dst->y += 360.f;
	if (dst->y >= 360.f)
		dst->y -= 360.f;

	dst->x = M_RadToDeg(atan2f(z1, src->y)) - 90.f;
	if (dst->x < 0.f)
		dst->x += 360.f;
	if (dst->x >= 360.f)
		dst->x -= 360.f;

	return dst;
}

static inline float
M_Vec3Distance(const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	struct NeVec3 diff;
	M_SubVec3(&diff, v2, v1);
	return fabsf(M_Vec3Length(&diff));
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
static inline struct NeVec3 *
M_Vec3Direction(struct NeVec3 *dst, const struct NeVec3 *src, const struct NeVec3 *fwd)
{
	const float xr = M_DegToRad(src->x);
	const float yr = M_DegToRad(src->y);
	const float zr = M_DegToRad(src->z);

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

static inline struct NeVec3 *
M_Vec3ProjectPlane(struct NeVec3 *d, const struct NeVec3 *pt, const struct NePlane *pl)
{
	struct NeVec3 n;
	float distance;

	M_CopyVec3(&n, &pl->normal);

	distance = -M_DotVec3(&n, pt);
	M_ScaleVec3(&n, &n, distance);
	M_AddVec3(d, pt, &n);

	return d;
}

static inline struct NeVec3 *
M_Vec3Project(const struct NeVec3 *w, const struct NeVec3 *v, struct NeVec3 *p)
{
	struct NeVec3 u_w, u_v;
	float cos0;

	M_NormalizeVec3(&u_w, w);
	M_NormalizeVec3(&u_v, v);

	cos0 = M_DotVec3(&u_w, &u_v);

	M_ScaleVec3(p, &u_w, M_Vec3Length(w) * cos0);

	return p;
}

/*
 * Reflects a vector about a given surface normal. The surface
 * normal is assumed to be of unit length.
 */
static inline struct NeVec3 *
M_ReflectVec3(struct NeVec3 *dst, const struct NeVec3 *src, const struct NeVec3 *normal)
{
	struct NeVec3 tmp;
	return M_SubVec3(dst, src, M_ScaleVec3(&tmp, normal, 2.f * M_DotVec3(src, normal)));
}

/*
 * swaps the values in one vector with another
 * NB does not return a value unlike normal
 */
static inline void
M_SwapVec3(struct NeVec3 *a, struct NeVec3 *b)
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
M_Vec3Equal(const struct NeVec3 *a, const struct NeVec3 *b)
{
	return M_FloatEqual(a->x, b->x) && M_FloatEqual(a->y, b->y) && M_FloatEqual(a->z, b->z);
}

static inline struct NeVec3 *
M_MulVec3M3(struct NeVec3 *dst, const struct NeVec3 *v, const struct NeMat3 *m)
{
	dst->x = v->x * m->mat[0] + v->y * m->mat[3] + v->z * m->mat[6];
	dst->y = v->x * m->mat[1] + v->y * m->mat[4] + v->z * m->mat[7];
	dst->z = v->x * m->mat[2] + v->y * m->mat[5] + v->z * m->mat[8];

	return dst;
}

/*
 * Multiplies a matrix by vector (x, y, z, 1). The result
 * is stored in pOut. pOut is returned.
 */
static inline struct NeVec3 *
M_MulMatrixVec3(struct NeVec3 *dst, const struct NeMatrix *m, const struct NeVec3 *v)
{
	dst->x = v->x * m->m[0] + v->y * m->m[1] + v->z * m->m[2] + m->m[3];
	dst->y = v->x * m->m[4] + v->y * m->m[5] + v->z * m->m[6] + m->m[7];
	dst->z = v->x * m->m[8] + v->y * m->m[9] + v->z * m->m[10] + m->m[11];

	return dst;
}

/*
 * Multiplies vector (x, y, z, 1) by a given matrix. The result
 * is stored in pOut. pOut is returned.
 */
static inline struct NeVec3 *
M_MulVec3Matrix(struct NeVec3 *dst, const struct NeVec3 *v, const struct NeMatrix *m)
{
	dst->x = v->x * m->m[0] + v->y * m->m[4] + v->z * m->m[8] + m->m[12];
	dst->y = v->x * m->m[1] + v->y * m->m[5] + v->z * m->m[9] + m->m[13];
	dst->z = v->x * m->m[2] + v->y * m->m[6] + v->z * m->m[10] + m->m[14];

	return dst;
}

static inline struct NeVec3 *
M_Vec3Min(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	dst->x = M_Min(v1->x, v2->x);
	dst->y = M_Min(v1->y, v2->y);
	dst->z = M_Min(v1->z, v2->z);

	return dst;
}

static inline struct NeVec3 *
M_Vec3Max(struct NeVec3 *dst, const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	dst->x = M_Max(v1->x, v2->x);
	dst->y = M_Max(v1->y, v2->y);
	dst->z = M_Max(v1->z, v2->z);

	return dst;
}

static const struct NeVec3 M_Vec3PositiveZ = { 0, 0, 1 };
static const struct NeVec3 M_Vec3NegativeZ = { 0, 0, -1 };
static const struct NeVec3 M_Vec3PositiveY = { 0, 1, 0 };
static const struct NeVec3 M_Vec3NegativeY = { 0, -1, 0 };
static const struct NeVec3 M_Vec3NegativeX = { -1, 0, 0 };
static const struct NeVec3 M_Vec3PositiveX = { 1, 0, 0 };

#endif /* _NE_MATH_VEC3_H_ */

/* NekoEngine
 *
 * vec3.h
 * Author: Alexandru Naiman
 *
 * 3 component vector functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
