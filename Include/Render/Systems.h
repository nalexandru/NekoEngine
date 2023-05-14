#ifndef NE_RENDER_SYSTEMS_H
#define NE_RENDER_SYSTEMS_H

#include <Render/Types.h>
#include <Render/Model.h>

#include NE_ATOMIC_HDR

#ifdef __cplusplus
extern "C" {
#endif

#define RE_COLLECT_DRAWABLES	"Re_CollectDrawables"
#define RE_MORPH_MODELS			"Re_MorphModels"

struct NeDrawable
{
	NeBufferHandle indexBuffer, vertexBuffer;
	uint64_t vertexOffset;
	enum NeIndexType indexType;
	uint32_t firstIndex, indexCount;
	uint32_t vertexCount;
	struct NeMatrix mvp;
	const struct NeMaterial *material;
	float distance;
	uint32_t instanceId;
	const struct NeModelInstance *mi;
	uint64_t vertexAddress, materialAddress;
	struct NeBounds bounds;
	const struct NeMatrix *modelMatrix;
};

struct NeCollectDrawablesArgs
{
	struct NeMatrix vp;
	struct NeArray *opaqueDrawableArrays, *blendedDrawableArrays, *instanceArrays, blendedDrawables;
	uint32_t *instanceOffset;
	uint32_t maxDrawables, requiredDrawables, drawableCount;
	NE_ALIGN(16) NE_ATOMIC_UINT nextArray, totalDrawables, visibleDrawables;
	struct NeScene *s;
	struct NeVec3 camPos;
	struct NeFrustum camFrustum;
};

#ifdef __cplusplus
}
#endif

#endif /* NE_RENDER_SYSTEMS_H */

/* NekoEngine
 *
 * Systems.h
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
