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

struct NeArray f_destroyedResources[RE_NUM_FRAMES];

bool
Re_InitResourceDestructor(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (!Rt_InitArray(&f_destroyedResources[i], 50, sizeof(struct NeTDestroy), MH_Render))
			return false;
	return true;
}

void
Re_DestroyResources(void)
{
	const struct NeTDestroy *d;
	Rt_ArrayForEach(d, &f_destroyedResources[Re_frameId]) {
		if (d->type == DT_OBJECT)
			d->destroy(d->object);
		else
			d->destroyHandle(d->handle);
	}
	Rt_ClearArray(&f_destroyedResources[Re_frameId], false);
}

void
Re_TermResourceDestructor(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		const struct NeTDestroy *d;
		Rt_ArrayForEach(d, &f_destroyedResources[i]) {
			if (d->type == DT_OBJECT)
				d->destroy(d->object);
			else
				d->destroyHandle(d->handle);
		}
		Rt_TermArray(&f_destroyedResources[i]);
	}
}

static void DestroyBuffer(struct NeBuffer *buff) { Re_BkDestroyBuffer(buff); }
static void DestroyTexture(struct NeTexture *tex) { Re_BkDestroyTexture(tex); }

#define TDESTROY(x, func)								\
void Re_TDestroy ## x(struct x *obj) {					\
	struct NeTDestroy d =								\
	{													\
		.type = DT_OBJECT,								\
		.object = obj,									\
		.destroy = (void(*)(void*))(func)				\
	};													\
	Rt_ArrayAdd(&f_destroyedResources[Re_frameId], &d);	\
}

#define TDESTROYH(x, func)								\
void Re_TDestroyH ## x(x ## Handle h) {					\
	struct NeTDestroy d =								\
	{													\
		.type = DT_HANDLE,								\
		.handle = h,									\
		.destroy = (void(*)(void*))(func)				\
	};													\
	Rt_ArrayAdd(&f_destroyedResources[Re_frameId], &d);	\
}

TDESTROY(NeBuffer, DestroyBuffer)
TDESTROY(NeTexture, DestroyTexture)
TDESTROY(NeFramebuffer, Re_DestroyFramebuffer)
TDESTROY(NeAccelerationStructure, Re_DestroyAccelerationStructure)
TDESTROY(NeSampler, Re_DestroySampler)

TDESTROYH(NeBuffer, Re_DestroyBuffer)

/* NekoEngine
 *
 * DestroyResource.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
