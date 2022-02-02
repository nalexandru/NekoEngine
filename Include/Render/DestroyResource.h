#ifndef _NE_RENDER_DESTROY_RESOURCE_H_
#define _NE_RENDER_DESTROY_RESOURCE_H_

#include <Render/Types.h>

void Re_DestroyResources(void);

bool Re_InitResourceDestructor(void);
void Re_TermResourceDestructor(void);

#define DESTROY_PROTO(x) void Re_TDestroy ## x(struct x *)
#define DESTROYH_PROTO(x) void Re_TDestroyH ## x(x ## Handle)

DESTROY_PROTO(NeBuffer);
DESTROY_PROTO(NeTexture);
DESTROY_PROTO(NeFramebuffer);
DESTROY_PROTO(NeAccelerationStructure);
DESTROY_PROTO(NeSampler);

DESTROYH_PROTO(NeBuffer);

#define Re_Destroy(x) _Generic((x), \
	NeBufferHandle: Re_TDestroyHNeBuffer, \
	struct NeBuffer *: Re_TDestroyNeBuffer, \
	struct NeTexture *: Re_TDestroyNeTexture, \
	struct NeFramebuffer *: Re_TDestroyNeFramebuffer, \
	struct NeAccelerationStructure *: Re_TDestroyNeAccelerationStructure, \
	struct NeSampler *: Re_TDestroyNeSampler \
)(x)

#endif /* _NE_RENDER_DESTROY_RESOURCE_H_ */
