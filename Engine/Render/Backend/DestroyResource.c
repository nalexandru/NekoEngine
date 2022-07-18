#include <Render/Render.h>
#include <Render/Backend.h>
#include <Render/RayTracing.h>
#include <Runtime/Runtime.h>

#define DT_OBJECT	0
#define DT_HANDLE	1

struct NeTDestroy
{
	uint8_t type;
	union {
		uint16_t handle;
		void *object;
	};
	union {
		void (*destroy)(void *);
		void (*destroyHandle)(uint16_t);
	};
};

struct NeArray _destroyedResources[RE_NUM_FRAMES];

bool
Re_InitResourceDestructor(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (!Rt_InitArray(&_destroyedResources[i], 50, sizeof(struct NeTDestroy), MH_Render))
			return false;
	return true;
}

void
Re_DestroyResources(void)
{
	const struct NeTDestroy *d;
	Rt_ArrayForEach(d, &_destroyedResources[Re_frameId]) {
		if (d->type == DT_OBJECT)
			d->destroy(d->object);
		else
			d->destroyHandle(d->handle);
	}
	Rt_ClearArray(&_destroyedResources[Re_frameId], false);
}

void
Re_TermResourceDestructor(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		const struct NeTDestroy *d;
		Rt_ArrayForEach(d, &_destroyedResources[i]) {
			if (d->type == DT_OBJECT)
				d->destroy(d->object);
			else
				d->destroyHandle(d->handle);
		}
		Rt_TermArray(&_destroyedResources[i]);
	}
}

static void _DestroyBuffer(struct NeBuffer *buff) { Re_BkDestroyBuffer(buff); }
static void _DestroyTexture(struct NeTexture *tex) { Re_BkDestroyTexture(tex); }

#define TDESTROY(x, func)								\
void Re_TDestroy ## x(struct x *obj) {					\
	struct NeTDestroy d =								\
	{													\
		.type = DT_OBJECT,								\
		.object = obj,									\
		.destroy = (void(*)(void*))func					\
	};													\
	Rt_ArrayAdd(&_destroyedResources[Re_frameId], &d);	\
}

#define TDESTROYH(x, func)								\
void Re_TDestroyH ## x(x ## Handle h) {					\
	struct NeTDestroy d =								\
	{													\
		.type = DT_HANDLE,								\
		.handle = h,									\
		.destroy = (void(*)(void*))func					\
	};													\
	Rt_ArrayAdd(&_destroyedResources[Re_frameId], &d);	\
}

TDESTROY(NeBuffer, _DestroyBuffer)
TDESTROY(NeTexture, _DestroyTexture)
TDESTROY(NeFramebuffer, Re_DestroyFramebuffer)
TDESTROY(NeAccelerationStructure, Re_DestroyAccelerationStructure)
TDESTROY(NeSampler, Re_DestroySampler)

TDESTROYH(NeBuffer, Re_DestroyBuffer)
