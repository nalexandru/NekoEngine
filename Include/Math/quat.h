#ifndef _NE_MATH_QUATERNION_H_
#define _NE_MATH_QUATERNION_H_

#include <Math/defs.h>
#include <Math/vec3.h>

#ifndef _NE_MATH_MAT4_H_
#	include <Math/vec4.h>
#endif

static inline struct NeQuaternion *
M_Quat(struct NeQuaternion *q, float x, float y, float z, float w)
{
	return (struct NeQuaternion *)M_Vec4((struct NeVec4 *)q, x, y, z, w);
}

static inline struct NeQuaternion *
M_CopyQuat(struct NeQuaternion *dst, const struct NeQuaternion *src)
{
	return (struct NeQuaternion *)M_CopyVec4((struct NeVec4 *)dst, (struct NeVec4 *)src);
}

static inline struct NeQuaternion *
M_QuatIdentity(struct NeQuaternion *q)
{
	return (struct NeQuaternion *)M_Vec4((struct NeVec4 *)q, 0.f, 0.f, 0.f, 1.f);
}

static inline float
M_QuatLengthSquared(const struct NeQuaternion *q)
{
	return M_Vec4LengthSquared((const struct NeVec4 *)q);
}

static inline float
M_QuatLength(const struct NeQuaternion *q)
{
	return M_Vec4Length((const struct NeVec4 *)q);
}

static inline struct NeQuaternion *
M_NormalizeQuat(struct NeQuaternion *dst, const struct NeQuaternion *src)
{
	return (struct NeQuaternion *)M_NormalizeVec4((struct NeVec4 *)dst, (struct NeVec4 *)src);
}

static inline struct NeQuaternion *
M_AddQuat(struct NeQuaternion *dst, const struct NeQuaternion *q1, const struct NeQuaternion *q2)
{
	return (struct NeQuaternion *)M_AddVec4((struct NeVec4 *)dst, (struct NeVec4 *)q1, (struct NeVec4 *)q2);
}

static inline struct NeQuaternion *
M_SubQuat(struct NeQuaternion *dst, const struct NeQuaternion *q1, const struct NeQuaternion *q2)
{
	return (struct NeQuaternion *)M_SubVec4((struct NeVec4 *)dst, (struct NeVec4 *)q1, (struct NeVec4 *)q2);
}

static inline struct NeQuaternion *
M_MulQuat(struct NeQuaternion *dst, const struct NeQuaternion *qu1, const struct NeQuaternion *qu2)
{
	struct NeQuaternion q1, q2;

	M_CopyQuat(&q1, qu1);
	M_CopyQuat(&q2, qu2);

	dst->x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
	dst->y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
	dst->z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;
	dst->w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;

	return dst;
}

static inline float
M_DotQuat(const struct NeQuaternion *q1, const struct NeQuaternion *q2)
{
	return M_DotVec4((struct NeVec4 *)q1, (struct NeVec4 *)q2);
}

static inline struct NeQuaternion *
M_ScaleQuat(struct NeQuaternion *dst, const struct NeQuaternion *src, float s)
{
	return (struct NeQuaternion *)M_MulVec4S((struct NeVec4 *)dst, (const struct NeVec4 *)src, s);
}

static inline struct NeQuaternion *
M_SlerpQuat(struct NeQuaternion *dst, const struct NeQuaternion *q1, const struct NeQuaternion *q2, float t)
{
	struct NeQuaternion tmp;
	struct NeQuaternion t1, t2;

	float theta_0;
	float theta;

	float dot = M_DotQuat(q1, q2);

	if (dot > 0.9995f) {
		M_SubQuat(&tmp, q2, q1);
		M_ScaleQuat(&tmp, &tmp, t);

		M_AddQuat(dst, q1, &tmp);
		return M_NormalizeQuat(dst, dst);
	}

	dot = M_ClampF(dot, -1.f, 1.f);

	theta_0 = acosf(dot);
	theta = theta_0 * t;

	M_ScaleQuat(&tmp, q1, dot);
	M_SubQuat(&tmp, q2, &tmp);
	M_NormalizeQuat(&tmp, &tmp);

	M_ScaleQuat(&t1, q1, cosf(theta));
	M_ScaleQuat(&t2, &tmp, sinf(theta));

	return M_AddQuat(dst, &t1, &t2);
}

static inline struct NeVec3 *
M_QuatMulVec3(struct NeVec3 *dst, const struct NeQuaternion *q, const struct NeVec3 *v)
{
	struct NeVec3 uv, uuv, qv;

	qv.x = q->x;
	qv.y = q->y;
	qv.z = q->z;
	
	M_CrossVec3(&uv, &qv, v);
	M_CrossVec3(&uuv, &qv, &uv);

	M_ScaleVec3(&uv, &uv, (2.f * q->w));
	M_ScaleVec3(&uuv, &uuv, 2.f);

	M_AddVec3(dst, v, &uv);

	return M_AddVec3(dst, dst, &uuv);
}

static inline struct NeQuaternion *
M_ConjugateQuat(struct NeQuaternion *dst, const struct NeQuaternion *src)
{
	struct NeVec4 inv = { -1.f, -1.f, -1.f, 1.f };
	return (struct NeQuaternion *)M_MulVec4((struct NeVec4 *)dst, (struct NeVec4 *)src, &inv);
}

static inline struct NeQuaternion *
M_QuatFromAxisAngleR(struct NeQuaternion *q, const struct NeVec3 *v, float rad)
{
	struct NeVec3 axis;

	const float angle = rad * 0.5f;
	const float scale = sinf(angle);

	M_NormalizeVec3(&axis, v);

	q->x = axis.x * scale;
	q->y = axis.y * scale;
	q->z = axis.z * scale;
	q->w = cosf(angle);

	return q;
}

static inline struct NeQuaternion *
M_QuatFromAxisAngle(struct NeQuaternion *q, const struct NeVec3 *v, float deg)
{
	return M_QuatFromAxisAngleR(q, v, M_DegToRad(deg));
}

static inline struct NeQuaternion *
M_QuatRotationAxisAngleR(struct NeQuaternion *q, const struct NeVec3 *v, float rad)
{
	struct NeQuaternion tmp;
	M_QuatFromAxisAngleR(&tmp, v, rad);
	return M_NormalizeQuat(q, M_MulQuat(q, q, &tmp));
}

static inline struct NeQuaternion *
M_QuatRotationAxisAngle(struct NeQuaternion *q, const struct NeVec3 *v, float deg)
{
	return M_QuatRotationAxisAngleR(q, v, M_DegToRad(deg));
}

static inline struct NeQuaternion *
M_QuatRotationPitchYawRoll(struct NeQuaternion *q, float pitchd, float yawd, float rolld)
{
	const float pitch = M_DegToRad(pitchd) * .5f;
	const float yaw = M_DegToRad(yawd) * .5f;
	const float roll = M_DegToRad(rolld) * .5f;

	const float cx = cosf(pitch);
	const float sx = sinf(pitch);
	const float cy = cosf(yaw);
	const float sy = sinf(yaw);
	const float cz = cosf(roll);
	const float sz = sinf(roll);

	q->w = cx * cy * cz + sx * sy * sz;
	q->x = sx * cy * cz - cx * sy * sz;
	q->y = cx * sy * cz + sx * cy * sz;
	q->z = cx * cy * sz - sx * sy * cz;

	return q;
}

static inline struct NeQuaternion *
M_QuatRotationMat3(struct NeQuaternion *q, const struct NeMat3 *m)
{
	/*
	 *	Note: The OpenGL matrices are transposed from the description below
	 *	taken from the Matrix and Quaternion FAQ
	 *
	 *	if (mat[0] > mat[5] && mat[0] > mat[10]) {	// Column 0:
	 *		S  = sqrt(1.0 + mat[0] - mat[5] - mat[10]) * 2;
	 *		X = 0.25 * S;
	 *		Y = (mat[4] + mat[1]) / S;
	 *		Z = (mat[2] + mat[8]) / S;
	 *		W = (mat[9] - mat[6]) / S;
	 *	} else if (mat[5] > mat[10]) {			// Column 1:
	 *		S  = sqrt( 1.0 + mat[5] - mat[0] - mat[10]) * 2;
	 *		X = (mat[4] + mat[1]) / S;
	 *		Y = 0.25 * S;
	 *		Z = (mat[9] + mat[6]) / S;
	 *		W = (mat[2] - mat[8]) / S;
	 *	} else {					// Column 2:
	 *		S  = sqrt(1.0 + mat[10] - mat[0] - mat[5]) * 2;
	 *		X = (mat[2] + mat[8]) / S;
	 *		Y = (mat[9] + mat[6]) / S;
	 *		Z = 0.25 * S;
	 *		W = (mat[4] - mat[1]) / S;
	 *	}
	 */

	float x, y, z, w;
	float mat[16];
	float scale = 0.f;
	float diagonal = 0.f;

	/*
	 *	0 3 6
	 *	1 4 7
	 *	2 5 8
	 *
	 *	0 1 2 3
	 *	4 5 6 7
	 *	8 9 10 11
	 *	12 13 14 15
	 */

	memset(mat, 0x0, sizeof(mat));

	mat[0] = m->mat[0];
	mat[1] = m->mat[3];
	mat[2] = m->mat[6];
	mat[4] = m->mat[1];
	mat[5] = m->mat[4];
	mat[6] = m->mat[7];
	mat[8] = m->mat[2];
	mat[9] = m->mat[5];
	mat[10] = m->mat[8];
	mat[15] = 1.f;

	diagonal = mat[0] + mat[5] + mat[10] + 1.f;

	if (diagonal > FLT_EPSILON) {
		// Calculate the scale of the diagonal
		scale = sqrtf(diagonal) * 2.f;

		// Calculate the x, y, x and w of the quaternion through the respective equation
		x = (mat[9] - mat[6]) / scale;
		y = (mat[2] - mat[8]) / scale;
		z = (mat[4] - mat[1]) / scale;
		w = .25f * scale;
	} else {
		// If the first element of the diagonal is the greatest value
		if (mat[0] > mat[5] && mat[0] > mat[10]) {
			// Find the scale according to the first element, and double that value
			scale = sqrtf(1.f + mat[0] - mat[5] - mat[10]) * 2.f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = .25f * scale;
			y = (mat[4] + mat[1]) / scale;
			z = (mat[2] + mat[8]) / scale;
			w = (mat[9] - mat[6]) / scale;
		} else if (mat[5] > mat[10]) {
			// Else if the second element of the diagonal is the greatest value
			// Find the scale according to the second element, and double that value
			scale = sqrtf(1.f + mat[5] - mat[0] - mat[10]) * 2.f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = (mat[4] + mat[1]) / scale;
			y = .25f * scale;
			z = (mat[9] + mat[6]) / scale;
			w = (mat[2] - mat[8]) / scale;
		} else {
			// Else the third element of the diagonal is the greatest value
			// Find the scale according to the third element, and double that value
			scale = sqrtf(1.f + mat[10] - mat[0] - mat[5]) * 2.f;

			// Calculate the x, y, x and w of the quaternion through the respective equation
			x = (mat[2] + mat[8]) / scale;
			y = (mat[9] + mat[6]) / scale;
			z = .25f * scale;
			w = (mat[4] - mat[1]) / scale;
		}
	}

	q->x = x;
	q->y = y;
	q->z = z;
	q->w = w;

	return q;
}

static inline struct NeQuaternion *
M_QuatLookAt(struct NeQuaternion *q, const struct NeVec3 *direction, const struct NeVec3 *up)
{
	struct NeMat3 rot;
	struct NeVec3 tmp;

	M_MulVec3S((struct NeVec3 *)&rot.mat[6], M_NormalizeVec3(&tmp, direction), -1);
	M_NormalizeVec3((struct NeVec3 *)&rot.mat[0], M_CrossVec3(&tmp, up, (struct NeVec3 *)&rot.mat[6]));
	M_CrossVec3((struct NeVec3 *)&rot.mat[3], (struct NeVec3 *)&rot.mat[6], (struct NeVec3 *)&rot.mat[0]);

	M_QuatRotationMat3(q, &rot);
	return M_NormalizeQuat(q, q);
}

static inline struct NeVec3 *
M_QuatUp(struct NeVec3 *v, const struct NeQuaternion *q)
{
	 return M_QuatMulVec3(v, q, &M_Vec3PositiveY);
}

static inline struct NeVec3 *
M_QuatForwardRH(struct NeVec3 *v, const struct NeQuaternion *q)
{
	return M_QuatMulVec3(v, q, &M_Vec3NegativeZ);
}

static inline struct NeVec3 *
M_QuatForwardLH(struct NeVec3 *v, const struct NeQuaternion *q)
{
	return M_QuatMulVec3(v, q, &M_Vec3PositiveZ);
}

static inline struct NeVec3 *
M_QuatRight(struct NeVec3 *v, const struct NeQuaternion *q)
{
	return M_QuatMulVec3(v, q, &M_Vec3PositiveX);
}

static inline float
M_QuatRoll(const struct NeQuaternion *q)
{
	const struct NeVec2 v0 = { 0.f, 0.f };
	const struct NeVec2 v =
	{
		2.f * (q->x * q->y + q->w * q->z),
		q->w * q->w + q->x * q->x - q->y * q->y - q->z * q->z
	};

	if (M_Vec2Equal(&v, &v0))
		return 0.f;

	return M_RadToDeg(atan2f(v.x, v.y));
}

static inline float
M_QuatPitch(const struct NeQuaternion *q)
{
	const struct NeVec2 v0 = { 0.f, 0.f };
	const struct NeVec2 v =
	{
		2.f * (q->y * q->z + q->w * q->x),
		q->w * q->w - q->x * q->x - q->y * q->y + q->z * q->z
	};

	if (M_Vec2Equal(&v, &v0))
		return M_RadToDeg(2.f * atan2f(q->x, q->w));

	return M_RadToDeg(atan2f(v.x, v.y));
}

static inline float
M_QuatYaw(const struct NeQuaternion *q)
{
	const float a = M_ClampF(-2.f * (q->x * q->z - q->w * q->y), -1.f, 1.f);
	return M_RadToDeg(asinf(a));
}

/*
 * Get the axis and angle of rotation from a quaternion
 */
static inline void
M_QuatToAxisAngle(const struct NeQuaternion *q, struct NeVec3 *axis, float *angle)
{
	float scale;
	struct NeQuaternion tmp;

	if (q->w > 1.f)
		M_NormalizeQuat(&tmp, q);
	else
		M_CopyQuat(&tmp, q);

	*angle = 2.f * acosf(tmp.w);
	scale = sqrtf(1.f - (tmp.w * tmp.w));

	// angle is 0 or 360 so just simply set axis to 0,0,1 with angle 0
	if (scale < FLT_EPSILON) {
		axis->x = 0.f;
		axis->y = 0.f;
		axis->z = 1.f;
	} else {
		axis->x = tmp.x / scale;
		axis->y = tmp.y / scale;
		axis->z = tmp.z / scale;
		M_NormalizeVec3(axis, axis);
	}
}

/*
 * Given a quaternion, and an axis. This extracts the rotation around
 * the axis into pOut as another quaternion. Uses the swing-twist
 * decomposition.
 */
static inline struct NeQuaternion *
M_QuatExtractRotation(const struct NeQuaternion *q, const struct NeVec3 *axis, struct NeQuaternion *dst)
{
	/*
	 * http://stackoverflow.com/questions/3684269/component-of-a-quaternion-rotation-around-an-axis/22401169?noredirect=1#comment34098058_22401169
	 */
	struct NeVec3 qv;
	float d;
	
	qv.x = q->x;
	qv.y = q->y;
	qv.z = q->z;
	
	d = M_DotVec3(&qv, axis);

	M_Quat(dst, axis->x * d, axis->y * d, axis->z * d, q->w);
	return M_NormalizeQuat(dst, dst);
}

/*
 * Returns a Quaternion representing the angle between two vectors
 */
static inline struct NeQuaternion *
M_QuatBetweenVec3(struct NeQuaternion *dst, const struct NeVec3 *u, const struct NeVec3 *v)
{
	struct NeVec3 w;
	float len;
	struct NeQuaternion q;

	if (M_Vec3Equal(u, v))
		return M_QuatIdentity(dst);

	len = sqrtf(M_Vec3LengthSquared(u) * M_Vec3LengthSquared(v));
	M_CrossVec3(&w, u, v);

	M_Quat(&q, w.x, w.y, w.z, M_DotVec3(u, v) + len);
	return M_NormalizeQuat(dst, &q);
}

/*
 * Gets the shortest arc quaternion to rotate this vector to the
 * destination vector.
 *
 * If you call this with a dest vector that is close to the inverse of
 * this vector, we will rotate 180 degrees around the 'fallbackAxis'
 * (if specified, or a generated axis if not) since in this case ANY
 * axis of rotation is valid.
 */
static inline struct NeQuaternion *
M_QuatRotationBetweenVec3(struct NeQuaternion *q,
	const struct NeVec3 *vec1, const struct NeVec3 *NeVec2,
	const struct NeVec3 *fallback)
{
	struct NeVec3 v1, v2;
	float a;

	M_CopyVec3(&v1, vec1);
	M_CopyVec3(&v2, NeVec2);

	M_NormalizeVec3(&v1, &v1);
	M_NormalizeVec3(&v2, &v2);

	a = M_DotVec3(&v1, &v2);

	if (a >= 1.f)
		return M_QuatIdentity(q);

	if (a < (1e-6f - 1.f)) {
		if (fabsf(M_Vec3LengthSquared(fallback)) < FLT_EPSILON) {
			M_QuatRotationAxisAngleR(q, fallback, PI);
		} else {
			struct NeVec3 axis;
			struct NeVec3 X;
			X.x = 1.f;
			X.y = 0.f;
			X.z = 0.f;

			M_CrossVec3(&axis, &X, vec1);

			// If axis is zero
			if (fabsf(M_Vec3LengthSquared(&axis)) < FLT_EPSILON) {
				struct NeVec3 Y;
				Y.x = 0.f;
				Y.y = 1.f;
				Y.z = 0.f;

				M_CrossVec3(&axis, &Y, vec1);
			}

			M_NormalizeVec3(&axis, &axis);

			M_QuatRotationAxisAngleR(q, &axis, PI);
		}
	} else {
		float s = sqrtf((1.f + a) * 2.f);
		float invs = 1.f / s;

		struct NeVec3 c;
		M_CrossVec3(&c, &v1, &v2);

		q->x = c.x * invs;
		q->y = c.y * invs;
		q->z = c.z * invs;
		q->w = s * .5f;

		M_NormalizeQuat(q, q);
	}

	return q;
}

#endif /* _NE_MATH_QUATERNION_H_ */

/* NekoEngine
 *
 * quat.h
 * Author: Alexandru Naiman
 *
 * Quaternion functions
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
