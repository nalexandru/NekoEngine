#include <Render/Render.h>
#include <Render/Driver/RayTracing.h>
#include <Runtime/Runtime.h>

#define DT_OBJECT	0
#define DT_HANDLE	1

struct TDestroy
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

struct Array _destroyedResources[RE_NUM_FRAMES];

bool
Re_InitResourceDestructor(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (!Rt_InitArray(&_destroyedResources[i], 50, sizeof(struct TDestroy), MH_Render))
			return false;
	return true;
}

void
Re_DestroyResources(void)
{
	const struct TDestroy *d;
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
		const struct TDestroy *d;
		Rt_ArrayForEach(d, &_destroyedResources[i]) {
			if (d->type == DT_OBJECT)
				d->destroy(d->object);
			else
				d->destroyHandle(d->handle);
		}
		Rt_TermArray(&_destroyedResources[i]);
	}
}

static void _DestroyTexture(struct Texture *tex) { Re_deviceProcs.DestroyTexture(Re_device, tex); }

#define TDESTROY(x, func)								\
void Re_TDestroy ## x(struct x *obj) {					\
	struct TDestroy d =									\
	{													\
		.type = DT_OBJECT,								\
		.object = obj,									\
		.destroy = (void(*)(void*))func					\
	};													\
	Rt_ArrayAdd(&_destroyedResources[Re_frameId], &d);	\
}

#define TDESTROYH(x, func)								\
void Re_TDestroy ## x(x ## Handle h) {					\
	struct TDestroy d =									\
	{													\
		.type = DT_HANDLE,								\
		.handle = h,									\
		.destroy = (void(*)(void*))func					\
	};													\
	Rt_ArrayAdd(&_destroyedResources[Re_frameId], &d);	\
}

TDESTROYH(Buffer, Re_DestroyBuffer)
TDESTROY(Texture, _DestroyTexture)
TDESTROY(Framebuffer, Re_DestroyFramebuffer)
TDESTROY(AccelerationStructure, Re_DestroyAccelerationStructure)
TDESTROY(Sampler, Re_DestroySampler)
