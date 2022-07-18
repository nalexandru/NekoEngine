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

#define M_Matrix			M_Matrix_SIMD
#define M_MatrixF			M_MatrixF_SIMD
#define M_CopyMatrix		M_CopyMatrix_SIMD
#define M_MulMatrix			M_MulMatrix_SIMD
#define M_MulMatrixS		M_MulMatrixS_SIMD
#define M_MulVec4Matrix		M_MulVec4MatrixSIMD
#define M_MulMatrixVec4		M_MulMatrixVec4SIMD

#ifndef M4_TRANSPOSE_NOSIMD
#define M_TransposeMatrix	M_TransposeMatrix_SIMD
#endif

#ifndef M4_INVERSE_NOSIMD
#define M_InverseMatrix		M_InverseMatrix_SIMD
#endif

#else

static inline struct NeMatrix *
M_Matrix(struct NeMatrix *dst, const float *m)
{
	memcpy(dst->m, m, sizeof(float) * 16);
	return dst;
}

static inline struct NeMatrix *
M_MatrixF(struct NeMatrix *dst,
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

static inline struct NeMatrix *
M_CopyMatrix(struct NeMatrix *dst, const struct NeMatrix *src)
{
	memcpy(dst->m, src->m, sizeof(float) * 16);
	return dst;
}

static inline struct NeMatrix *
M_MulMatrix(struct NeMatrix *dst, const struct NeMatrix *m1, const struct NeMatrix *m2)
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

static inline struct NeMatrix *
M_MulMatrixS(struct NeMatrix *dst, const struct NeMatrix *m, const float f)
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
static inline struct NeMatrix *
M_InverseMatrix(struct NeMatrix *dst, const struct NeMatrix *src)
{
	struct NeMatrix tmp;
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
static inline struct NeMatrix *
M_TransposeMatrix(struct NeMatrix *dst, const struct NeMatrix *src)
{
	struct NeMatrix tmp;
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
M_MulVec4Matrix(struct vec4 *dst, const struct vec4 *v, const struct NeMatrix *m)
{
	dst->x = v->x * m->m[0] + v->y * m->m[1] + v->z * m->m[2] + v->w * m->m[3];
	dst->y = v->x * m->m[4] + v->y * m->m[5] + v->z * m->m[6] + v->w * m->m[7];
	dst->z = v->x * m->m[8] + v->y * m->m[9] + v->z * m->m[10] + v->w * m->m[11];
	dst->w = v->x * m->m[12] + v->y * m->m[13] + v->z * m->m[14] + v->w * m->m[15];

	return dst;
}

static inline struct vec4 *
M_MulMatrixVec4(struct vec4 *dst, const struct NeMatrix *m, const struct vec4 *v)
{
	dst->x = v->x * m->m[0] + v->y * m->m[1] + v->z * m->m[2] + v->w * m->m[3];
	dst->y = v->x * m->m[4] + v->y * m->m[5] + v->z * m->m[6] + v->w * m->m[7];
	dst->z = v->x * m->m[8] + v->y * m->m[9] + v->z * m->m[10] + v->w * m->m[11];
	dst->w = v->x * m->m[12] + v->y * m->m[13] + v->z * m->m[14] + v->w * m->m[15];

	return dst;
}
#endif

static inline struct NeMatrix *
M_MatrixIdentity(struct NeMatrix *m)
{
	float md[16] = { 0.f };
	md[0] = md[5] = md[10] = md[15] = 1.f;
	return M_Matrix(m, md);
}

static inline struct NeMatrix *
M_RotationMatrixX(struct NeMatrix *dst, const float rad)
{
	return M_MatrixF(dst,
		1.f, 0.f, 0.f, 0.f,
		0.f, cosf(rad), sinf(rad), 0.f,
		0.f, -sinf(rad), cosf(rad), 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct NeMatrix *
M_RotationMatrixY(struct NeMatrix *dst, const float rad)
{
	return M_MatrixF(dst,
		cosf(rad), 0.f, -sinf(rad), 0.f,
		0.f, 1.f, 0.f, 0.f,
		sinf(rad), 0.f, cosf(rad), 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct NeMatrix *
M_RotationMatrixZ(struct NeMatrix *dst, const float rad)
{
	return M_MatrixF(dst,
		cosf(rad), sinf(rad), 0.f, 0.f,
		-sinf(rad), cosf(rad), 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct NeMatrix *
M_RotationMatrixFromQuat(struct NeMatrix *dst, const struct NeQuaternion *q)
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

	return M_MatrixF(dst,
		1.f - 2.f * (yy + zz), 2.f * (xy + zw), 2.f * (xz - yw), 0.f,
		2.f * (xy - zw), 1.f - 2.f * (xx + zz), 2.f * (yz + xw), 0.f,
		2.f * (xz + yw), 2.f * (yz - xw), 1.f - 2.f * (xx + yy), 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct NeMatrix *
M_RotationMatrixFromAxisAngle(struct NeMatrix *dst, const struct NeVec3 *axis, float deg)
{
	struct NeQuaternion quat;
	M_QuatRotationAxisAngle(&quat, axis, deg);
	M_RotationMatrixFromQuat(dst, &quat);
	return dst;
}

static inline struct NeMatrix *
M_RotationMatrixFromPitchYawRoll(struct NeMatrix *dst, const float pitch, const float yaw, const float roll)
{
	struct NeMatrix yaw_matrix;
	struct NeMatrix roll_matrix;
	struct NeMatrix pitch_matrix;

	M_RotationMatrixY(&yaw_matrix, yaw);
	M_RotationMatrixX(&pitch_matrix, pitch);
	M_RotationMatrixZ(&roll_matrix, roll);

	M_MulMatrix(dst, &pitch_matrix, &roll_matrix);
	M_MulMatrix(dst, &yaw_matrix, dst);

	return dst;
}

static inline struct NeMatrix *
M_MatrixLookAt(struct NeMatrix *dst, const struct NeVec3 *eye, const struct NeVec3 *center, const struct NeVec3 *up)
{
	struct NeVec3 f;
	struct NeVec3 s;
	struct NeVec3 u;

	M_SubVec3(&f, center, eye);
	M_NormalizeVec3(&f, &f);

	M_CrossVec3(&s, &f, up);
	M_NormalizeVec3(&s, &s);

	M_CrossVec3(&u, &s, &f);

	return M_MatrixF(dst,
		s.x, u.x, -f.x, 0.f,
		s.y, u.y, -f.y, 0.f,
		s.z, u.z, -f.z, 0.f,
		-M_DotVec3(&s, eye), -M_DotVec3(&u, eye), M_DotVec3(&f, eye), 1.f);
}


static inline struct NeMatrix *
M_ScaleMatrix(struct NeMatrix *dst, const float x, const float y, const float z)
{
	return M_MatrixF(dst,
		  x, 0.f, 0.f, 0.f,
		0.f,   y, 0.f, 0.f,
		0.f, 0.f,   z, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

static inline struct NeMatrix *
M_ScaleMatrixV(struct NeMatrix *dst, const struct NeVec3 *v)
{
	return M_ScaleMatrix(dst, v->x, v->y, v->z);
}

static inline struct NeMatrix *
M_TranslationMatrix(struct NeMatrix *dst, const float x, const float y, const float z)
{
	return M_MatrixF(dst,
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		  x,   y,   z, 1.f);
}

static inline struct NeMatrix *
M_TranslationMatrixV(struct NeMatrix *dst, const struct NeVec3 *v)
{
	return M_TranslationMatrix(dst, v->x, v->y, v->z);
}

static inline struct NeVec3 *
M_MatrixUp(struct NeVec3 *v, const struct NeMatrix *m)
{
	M_MulVec3Matrix(v, &M_Vec3PositiveY, m);
	return M_NormalizeVec3(v, v);
}

static inline struct NeVec3 *
M_MatrixForwardRH(struct NeVec3 *v, const struct NeMatrix *m)
{
	M_MulVec3Matrix(v, &M_Vec3NegativeZ, m);
	return M_NormalizeVec3(v, v);
}

static inline struct NeVec3 *
M_MatrixForwardLH(struct NeVec3 *v, const struct NeMatrix *m)
{
	M_MulVec3Matrix(v, &M_Vec3PositiveZ, m);
	return M_NormalizeVec3(v, v);
}

static inline struct NeVec3 *
M_MatrixRight(struct NeVec3 *v, const struct NeMatrix *m)
{
	M_MulVec3Matrix(v, &M_Vec3PositiveX, m);
	return M_NormalizeVec3(v, v);
}

static inline struct NeMatrix *
M_PerspectiveMatrix(struct NeMatrix *dst, float fov_y, float aspect, float z_near, float z_far)
{
	const float rad = 0.5f * M_DegToRad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);
	const float w = h / aspect;

	const float m22 = z_far / (z_near - z_far);
	const float m32 = -(z_far * z_near) / (z_far - z_near);

	return M_MatrixF(dst,
		  w, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, m22, -1.f,
		0.f, 0.f, m32,  0.f);
}

static inline struct NeMatrix *
M_PerspectiveMatrixND(struct NeMatrix *dst, float fov_y, float aspect, float z_near, float z_far)
{
	const float rad = 0.5f * M_DegToRad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);
	const float w = h / aspect;

	const float m22 = -(z_far * z_near) / (z_far - z_near);
	const float m32 = -(2.f * z_far * z_near) / (z_far - z_near);

	return M_MatrixF(dst,
		  w, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, m22, -1.f,
		0.f, 0.f, m32,  0.f);
}

static inline struct NeMatrix *
M_InfinitePerspectiveMatrixRZ(struct NeMatrix *dst, float fov_y, float aspect, float z_near)
{
	const float rad = 0.5f * M_DegToRad(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);

	return M_MatrixF(dst,
		h / aspect, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, 0.f, -1.f,
		0.f, 0.f, z_near,  0.f);
}

static inline struct NeMatrix *
M_OrthographicMatrix(struct NeMatrix *dst, float left, float right, float bottom, float top, float z_near, float z_far)
{
	return M_MatrixF(dst,
		2.f / (right - left), 0.f, 0.f,  0.f,
		0.f, 2.f / (top - bottom), 0.f,  0.f,
		0.f, 0.f, 1.f / (z_far - z_near), 0.f,
		-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -dst->r[2][2] * z_near, 1.f);
}

static inline struct NeMatrix *
M_OrthographicMatrixND(struct NeMatrix *dst, float left, float right, float bottom, float top, float z_near, float z_far)
{
	return M_MatrixF(dst,
		2.f / (right - left), 0.f, 0.f,  0.f,
		0.f, 2.f / (top - bottom), 0.f,  0.f,
		0.f, 0.f, -2.f / (z_far - z_near), 0.f,
		-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((z_far + z_near) / (z_far - z_near)), 1.f);
}

#endif /* _NE_MATH_MAT4_H_ */

/* NekoEngine
 *
 * mat4.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions
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
