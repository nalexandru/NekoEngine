/* NekoEngine
 *
 * quat.h
 * Author: Alexandru Naiman
 *
 * Quaternion functions
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

#ifndef _NE_MATH_QUATERNION_H_
#define _NE_MATH_QUATERNION_H_

#include <Math/defs.h>
#include <Math/vec3.h>

static inline struct quat *
quat(struct quat *q, float x, float y, float z, float w)
{
	q->x = x;
	q->y = y;
	q->z = z;
	q->w = w;

	return q;
}

static inline struct quat *
quat_copy(struct quat *dst, const struct quat *src)
{
	memcpy(dst, src, sizeof(float) * 4);
	return dst;
}

static inline struct quat *
quat_ident(struct quat *q)
{
	q->x = 0.f;
	q->y = 0.f;
	q->z = 0.f;
	q->w = 1.f;

	return q;
}

static inline float
quat_len_sq(const struct quat *q)
{
	return (q->x * q->x) + (q->y * q->y) + (q->z * q->z) + (q->w * q->w);
}

static inline float
quat_len(const struct quat *q)
{
	return sqrtf((q->x * q->x) + (q->y * q->y) + (q->z * q->z) + (q->w * q->w));
}

static inline struct quat *
quat_norm(struct quat *dst, const struct quat *src)
{
	float length = quat_len(src);

	if (fabsf(length) < FLT_EPSILON) {
		dst->x = 0.f;
		dst->y = 0.f;
		dst->z = 0.f;
		dst->w = 1.f;

		return dst;
	}

	return quat(dst,
		src->x / length,
		src->y / length,
		src->z / length,
		src->w / length);
}

static inline struct quat *
quat_add(struct quat *dst, const struct quat *q1, const struct quat *q2)
{
	dst->x = q1->x + q2->x;
	dst->y = q1->y + q2->y;
	dst->z = q1->z + q2->z;
	dst->w = q1->w + q2->w;

	return dst;
}

static inline struct quat *
quat_sub(struct quat *dst, const struct quat *q1, const struct quat *q2)
{
	dst->x = q1->x - q2->x;
	dst->y = q1->y - q2->y;
	dst->z = q1->z - q2->z;
	dst->w = q1->w - q2->w;

	return dst;
}

static inline struct quat *
quat_mul(struct quat *dst, const struct quat *qu1, const struct quat *qu2)
{
	struct quat *q1 = NULL;
	struct quat *q2 = NULL;
	struct quat tmp1, tmp2;

	quat_copy(&tmp1, qu1);
	quat_copy(&tmp2, qu2);

	// Just aliasing
	q1 = &tmp1;
	q2 = &tmp2;

	dst->x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
	dst->y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
	dst->z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
	dst->w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;

	return dst;
}

static inline float
quat_dot(const struct quat *q1, const struct quat *q2)
{
	/*
	 * A dot B = B dot A = AtBt + AxBx + AyBy + AzBz
	 */
	return q1->w *q2->w + q1->x * q2->x + q1->y * q2->y + q1->z * q2->z;
}

static inline struct quat *
quat_scale(struct quat *dst, const struct quat *src, float s)
{
	dst->x = src->x * s;
	dst->y = src->y * s;
	dst->z = src->z * s;
	dst->w = src->w * s;

	return dst;
}

static inline struct quat *
quat_slerp(struct quat *dst, const struct quat *q1, const struct quat *q2, float t)
{
	struct quat tmp;
	struct quat t1, t2;

	float theta_0;
	float theta;

	float dot = quat_dot(q1, q2);

	if (dot > 0.9995f) {
		quat_sub(&tmp, q2, q1);
		quat_scale(&tmp, &tmp, t);

		quat_add(dst, q1, &tmp);
		return quat_norm(dst, dst);
	}

	dot = clamp(dot, -1.f, 1.f);

	theta_0 = acosf(dot);
	theta = theta_0 * t;

	quat_scale(&tmp, q1, dot);
	quat_sub(&tmp, q2, &tmp);
	quat_norm(&tmp, &tmp);

	quat_scale(&t1, q1, cosf(theta));
	quat_scale(&t2, &tmp, sinf(theta));

	return quat_add(dst, &t1, &t2);
}

static inline struct vec3 *
quat_mul_vec3(struct vec3 *dst, const struct quat *q, const struct vec3 *v)
{
	struct vec3 uv, uuv, qv;

	qv.x = q->x;
	qv.y = q->y;
	qv.z = q->z;
	
	v3_cross(&uv, &qv, v);
	v3_cross(&uuv, &qv, &uv);

	v3_scale(&uv, &uv, (2.f * q->w));
	v3_scale(&uuv, &uuv, 2.f);

	v3_add(dst, v, &uv);
	return v3_add(dst, dst, &uuv);
}

static inline struct quat *
quat_inverse(struct quat *dst, const struct quat *src)
{
	dst->x = -src->x;
	dst->y = -src->y;
	dst->z = -src->z;
	dst->w = src->w;

	return dst;
}

static inline struct quat *
quat_rot_axis_angle_r(struct quat *q, const struct vec3 *v, float rad)
{
	struct quat tmp;
	const float scale = sinf(rad * .5f);

	tmp.x = v->x * scale;
	tmp.y = v->y * scale;
	tmp.z = v->z * scale;
	tmp.w = cosf(rad * .5f);

	quat_mul(q, q, &tmp);

	return quat_norm(q, q);
}

static inline struct quat *
quat_rot_axis_angle(struct quat *q, const struct vec3 *v, float deg)
{
	return quat_rot_axis_angle_r(q, v, deg_to_rad(deg));
}

static inline struct quat *
quat_rot_pitch_yaw_roll(struct quat *q, float rolld, float pitchd, float yawd)
{
	const float pitch = deg_to_rad(pitchd);
	const float yaw = deg_to_rad(yawd);
	const float roll = deg_to_rad(rolld);

	const float cy = cosf(yaw * .5f);
	const float sy = sinf(yaw * .5f);
	const float cp = cosf(pitch * .5f);
	const float sp = sinf(pitch * .5f);
	const float cr = cosf(roll * .5f);
	const float sr = sinf(roll * .5f);

	q->w = cr * cp * cy + sr * sp * sy;
	q->x = sr * cp * cy - cr * sp * sy;
	q->y = cr * sp * cy + sr * cp * sy;
	q->z = cr * cp * sy - sr * sp * cy;

	return q;
}

static inline struct quat *
quat_rot_m3(struct quat *q, const struct mat3 *m)
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

static inline struct quat *
quat_look_at(struct quat *q, const struct vec3 *direction, const struct vec3 *up)
{
	struct mat3 rot;
	struct vec3 tmp;

	v3_muls((struct vec3 *)&rot.mat[6], v3_norm(&tmp, direction), -1);
	v3_norm((struct vec3 *)&rot.mat[0], v3_cross(&tmp, up, (struct vec3 *)&rot.mat[6]));
	v3_cross((struct vec3 *)&rot.mat[3], (struct vec3 *)&rot.mat[6], (struct vec3 *)&rot.mat[0]);

	quat_rot_m3(q, &rot);
	return quat_norm(q, q);
}

static inline struct vec3 *
quat_up(struct vec3 *v, const struct quat *q)
{
	 return quat_mul_vec3(v, q, &KM_VEC3_POS_Y);
}

static inline struct vec3 *
quat_fwd_rh(struct vec3 *v, const struct quat *q)
{
	return quat_mul_vec3(v, q, &KM_VEC3_NEG_Z);
}

static inline struct vec3 *
quat_fwd_lh(struct vec3 *v, const struct quat *q)
{
	return quat_mul_vec3(v, q, &KM_VEC3_POS_Z);
}

static inline struct vec3 *
quat_right(struct vec3 *v, const struct quat *q)
{
	return quat_mul_vec3(v, q, &KM_VEC3_POS_X);
}

static inline float
quat_roll(const struct quat *q)
{
	const float srcp = 2.f * (q->w * q->x + q->y * q->z);
	const float crcp = 1.f - 2.f * (q->x * q->x + q->y * q->y);
	
	return rad_to_deg(atan2f(srcp, crcp));
}

static inline float
quat_pitch(const struct quat *q)
{
	const float sp = 2.f * (q->w * q->y - q->z * q->x);

	return rad_to_deg(fabsf(sp) >= 1.f ? copysignf(PI / 2.f, sp) : asinf(sp));
}

static inline float
quat_yaw(const struct quat *q)
{
	const float sycp = 2.f * (q->w * q->z + q->x * q->y);
	const float cycp = 1.f - 2.f * (q->y * q->y + q->z * q->z);

	return rad_to_deg(atan2f(sycp, cycp));
}

/*
 * Get the axis and angle of rotation from a quaternion
 */
static inline void
quat_to_axis_angle(const struct quat *q, struct vec3 *axis, float *angle)
{
	float scale;
	struct quat tmp;

	if (q->w > 1.f)
		quat_norm(&tmp, q);
	else
		quat_copy(&tmp, q);

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
		v3_norm(axis, axis);
	}
}

/*
 * Given a quaternion, and an axis. This extracts the rotation around
 * the axis into pOut as another quaternion. Uses the swing-twist
 * decomposition.
 */
static inline struct quat *
quat_extract_rot(const struct quat *q, const struct vec3 *axis, struct quat *dst)
{
	/*
	 * http://stackoverflow.com/questions/3684269/component-of-a-quaternion-rotation-around-an-axis/22401169?noredirect=1#comment34098058_22401169
	 */
	struct vec3 qv;
	float d;
	
	qv.x = q->x;
	qv.y = q->y;
	qv.z = q->z;
	
	d = v3_dot(&qv, axis);

	quat(dst, axis->x * d, axis->y * d, axis->z * d, q->w);
	return quat_norm(dst, dst);
}

/*
 * Returns a Quaternion representing the angle between two vectors
 */
static inline struct quat *
quat_between_v3(struct quat *dst, const struct vec3 *u, const struct vec3 *v)
{
	struct vec3 w;
	float len;
	struct quat q;

	if (v3_equal(u, v))
		return quat_ident(dst);

	len = sqrtf(v3_len_sq(u) * v3_len_sq(v));
	v3_cross(&w, u, v);

	quat(&q, w.x, w.y, w.z, v3_dot(u, v) + len);
	return quat_norm(dst, &q);
}

/*
 *  Gets the shortest arc quaternion to rotate this vector to the
 *  destination vector.
 *
 * If you call this with a dest vector that is close to the inverse of
 * this vector, we will rotate 180 degrees around the 'fallbackAxis'
 * (if specified, or a generated axis if not) since in this case ANY
 * axis of rotation is valid.
 */
static inline struct quat *
quat_rot_between_v3(struct quat *q,
	const struct vec3 *vec1, const struct vec3 *vec2,
	const struct vec3 *fallback)
{
	struct vec3 v1, v2;
	float a;

	v3_copy(&v1, vec1);
	v3_copy(&v2, vec2);

	v3_norm(&v1, &v1);
	v3_norm(&v2, &v2);

	a = v3_dot(&v1, &v2);

	if (a >= 1.f)
		return quat_ident(q);

	if (a < (1e-6f - 1.f)) {
		if (fabsf(v3_len_sq(fallback)) < FLT_EPSILON) {
			quat_rot_axis_angle_r(q, fallback, PI);
		} else {
			struct vec3 axis;
			struct vec3 X;
			X.x = 1.f;
			X.y = 0.f;
			X.z = 0.f;

			v3_cross(&axis, &X, vec1);

			// If axis is zero
			if (fabsf(v3_len_sq(&axis)) < FLT_EPSILON) {
				struct vec3 Y;
				Y.x = 0.f;
				Y.y = 1.f;
				Y.z = 0.f;

				v3_cross(&axis, &Y, vec1);
			}

			v3_norm(&axis, &axis);

			quat_rot_axis_angle_r(q, &axis, PI);
		}
	} else {
		float s = sqrtf((1.f + a) * 2.f);
		float invs = 1.f / s;

		struct vec3 c;
		v3_cross(&c, &v1, &v2);

		q->x = c.x * invs;
		q->y = c.y * invs;
		q->z = c.z * invs;
		q->w = s * .5f;

		quat_norm(q, q);
	}

	return q;
}

#endif /* _NE_MATH_QUATERNION_H_ */

