#ifndef _RE_DESTROY_RESOURCE_H_
#define _RE_DESTROY_RESOURCE_H_

#include <Render/Types.h>

void Re_DestroyResources(void);

bool Re_InitResourceDestructor(void);
void Re_TermResourceDestructor(void);

#define DESTROY_PROTO(x) void Re_TDestroy ## x(struct x *)
#define DESTROYH_PROTO(x) void Re_TDestroy ## x(x ## Handle)

DESTROY_PROTO(Texture);
DESTROY_PROTO(Framebuffer);
DESTROY_PROTO(AccelerationStructure);
DESTROY_PROTO(Sampler);

DESTROYH_PROTO(Buffer);

#define Re_Destroy(x) _Generic((x), \
	BufferHandle: Re_TDestroyBuffer, \
	struct Texture *: Re_TDestroyTexture, \
	struct Framebuffer *: Re_TDestroyFramebuffer, \
	struct AccelerationStructure *: Re_TDestroyAccelerationStructure, \
	struct Sampler *: Re_TDestroySampler \
)(x)

#endif /* _RE_DESTROY_RESOURCE_H_ */

