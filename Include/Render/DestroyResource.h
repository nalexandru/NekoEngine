#ifndef _NE_RENDER_DESTROY_RESOURCE_H_
#define _NE_RENDER_DESTROY_RESOURCE_H_

#include <Render/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus

#define Re_Destroy(x) _Generic((x), \
	NeBufferHandle: Re_TDestroyHNeBuffer, \
	struct NeBuffer *: Re_TDestroyNeBuffer, \
	struct NeTexture *: Re_TDestroyNeTexture, \
	struct NeFramebuffer *: Re_TDestroyNeFramebuffer, \
	struct NeAccelerationStructure *: Re_TDestroyNeAccelerationStructure, \
	struct NeSampler *: Re_TDestroyNeSampler \
)(x)

#else

static inline void Re_Destroy(NeBufferHandle h) { Re_TDestroyHNeBuffer(h); }
static inline void Re_Destroy(struct NeBuffer *b) { Re_TDestroyNeBuffer(b); }
static inline void Re_Destroy(struct NeTexture *t) { Re_TDestroyNeTexture(t); }
static inline void Re_Destroy(struct NeFramebuffer *fb) { Re_TDestroyNeFramebuffer(fb); }
static inline void Re_Destroy(struct NeAccelerationStructure *as) { Re_TDestroyNeAccelerationStructure(as); }
static inline void Re_Destroy(struct NeSampler *s) { Re_TDestroyNeSampler(s); }

#endif

#endif /* _NE_RENDER_DESTROY_RESOURCE_H_ */

/* NekoEngine
 *
 * DestroyResource.h
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
 */
