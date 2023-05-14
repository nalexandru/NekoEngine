#ifndef NE_MATH_DEBUG_H
#define NE_MATH_DEBUG_H

#include <Math/Types.h>
#include <System/Log.h>

static inline float
M_LogFloat(float f, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: float(%.05f)", name, f);
	return f;
}

static inline double
M_LogDouble(double d, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: double(%.05f)", name, d);
	return d;
}

static inline struct NeVec2 *
M_LogVec2(struct NeVec2 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeVec2(%.05f, %.05f)", name, v->x, v->y);
	return v;
}

static inline const struct NeVec2 *
M_LogVec2C(const struct NeVec2 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeVec2(%.05f, %.05f)", name, v->x, v->y);
	return v;
}

static inline struct NeVec3 *
M_LogVec3(struct NeVec3 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeVe34(%.05f, %.05f, %.05f)", name, v->x, v->y, v->z);
	return v;
}

static inline const struct NeVec3 *
M_LogVec3C(const struct NeVec3 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeVe34(%.05f, %.05f, %.05f)", name, v->x, v->y, v->z);
	return v;
}

static inline struct NeVec4 *
M_LogVec4(struct NeVec4 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeVec4(%.05f, %.05f, %.05f, %.05f)", name, v->x, v->y, v->z, v->w);
	return v;
}

static inline const struct NeVec4 *
M_LogVec4C(const struct NeVec4 *v, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeVec4(%.05f, %.05f, %.05f, %.05f)", name, v->x, v->y, v->z, v->w);
	return v;
}

static inline struct NeQuaternion *
M_LogQuaternion(struct NeQuaternion *q, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeQuaternion(%.05f, %.05f, %.05f, %.05f)", name, q->x, q->y, q->z, q->w);
	return q;
}

static inline const struct NeQuaternion *
M_LogQuaternionC(const struct NeQuaternion *q, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeQuaternion(%.05f, %.05f, %.05f, %.05f)", name, q->x, q->y, q->z, q->w);
	return q;
}

static inline const NeMatrix *
M_LogMatrixC(const struct NeMatrix *m, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeMatrix\n\t%.05f, %.05f, %.05f, %.05f\n\t%.05f, %.05f, %.05f, %.05f" \
		"\n\t%.05f, %.05f, %.05f, %.05f\n\t%.05f, %.05f, %.05f, %.05f", name,
		m->m[0], m->m[1], m->m[2], m->m[3], m->m[4], m->m[5], m->m[6], m->m[7], m->m[8], m->m[9], m->m[10],
		m->m[11], m->m[12], m->m[13], m->m[14], m->m[15]);
	return m;
}

static inline struct NeMatrix *
M_LogMatrix(struct NeMatrix *m, const char *name, const char *module)
{
	Sys_LogEntry(module, LOG_DEBUG, "%s: NeMatrix\n\t%.09f, %.09f, %.09f, %.09f\n\t%.09f, %.09f, %.09f, %.09f" \
		"\n\t%.09f, %.09f, %.09f, %.09f\n\t%.09f, %.09f, %.09f, %.09f", name,
		m->m[0], m->m[1], m->m[2], m->m[3], m->m[4], m->m[5], m->m[6], m->m[7], m->m[8], m->m[9], m->m[10],
		m->m[11], m->m[12], m->m[13], m->m[14], m->m[15]);
	return m;
}

#ifndef __cplusplus

#define M_Log(x, y, z) _Generic((x), \
	float: M_LogFloat, \
	double: M_LogDouble, \
	struct NeVec2 *: M_LogVec2, \
	const struct NeVec2 *: M_LogVec2C, \
	struct NeVec3 *: M_LogVec3, \
	const struct NeVec3 *: M_LogVec3C, \
	struct NeVec4 *: M_LogVec4, \
	const struct NeVec4 *: M_LogVec4C, \
	struct NeQuaternion *: M_LogQuaternion, \
	const struct NeQuaternion *: M_LogQuaternionC, \
	struct NeMatrix *: M_LogMatrix, \
	const struct NeMatrix *: M_LogMatrixC, \
)(x, y, z)

#else

#include <Math/Math.h>

static inline float M_Log(float f, const char *name, const char *module) { return M_LogFloat(f, name, module); }
static inline double M_Log(double d, const char *name, const char *module) { return M_LogDouble(d, name, module); }
static inline struct NeVec2 *M_Log(struct NeVec2 *v, const char *name, const char *module) { return M_LogVec2(v, name, module); }
static inline const struct NeVec2 *M_Log(const struct NeVec2 *v, const char *name, const char *module) { return M_LogVec2C(v, name, module); }
static inline struct NeVec3 *M_Log(struct NeVec3 *v, const char *name, const char *module) { return M_LogVec3(v, name, module); }
static inline const struct NeVec3 *M_Log(const struct NeVec3 *v, const char *name, const char *module) { return M_LogVec3C(v, name, module); }
static inline struct NeVec4 *M_Log(struct NeVec4 *v, const char *name, const char *module) { return M_LogVec4(v, name, module); }
static inline const struct NeVec4 *M_Log(const struct NeVec4 *v, const char *name, const char *module) { return M_LogVec4C(v, name, module); }
static inline struct NeQuaternion *M_Log(struct NeQuaternion *v, const char *name, const char *module) { return M_LogQuaternion(v, name, module); }
static inline const struct NeQuaternion *M_Log(const struct NeQuaternion *v, const char *name, const char *module) { return M_LogQuaternionC(v, name, module); }
static inline struct NeMatrix *M_Log(struct NeMatrix *v, const char *name, const char *module) { return M_LogMatrix(v, name, module); }
static inline const struct NeMatrix *M_Log(const struct NeMatrix *v, const char *name, const char *module) { return M_LogMatrixC(v, name, module); }
static inline XMMATRIX &M_Log(XMMATRIX &m, const char *name, const char *module)
{
	struct NeMatrix nm;
	M_Store(&nm, m);
	M_LogMatrix(&nm, name, module);
	return m;
}
static inline const XMMATRIX &M_Log(const XMMATRIX &m, const char *name, const char *module)
{
	struct NeMatrix nm;
	M_Store(&nm, m);
	M_LogMatrix(&nm, name, module);
	return m;
}

#endif

#endif /* NE_MATH_DEBUG_H */