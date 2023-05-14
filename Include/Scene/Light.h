#ifndef NE_SCENE_LIGHT_H
#define NE_SCENE_LIGHT_H

#include <Math/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SCN_COLLECT_LIGHTS		"Scn_CollectLights"

enum NeLightType
{
	LT_Directional = 0,
	LT_Point = 1,
	LT_Spot = 2,

	LT_FORCE_UINT32 = 0xFFFFFFFF
};

struct NeLight
{
	NE_COMPONENT_BASE;

	struct NeVec3 color;
	float intensity;
	enum NeLightType type;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

#pragma pack(push, 1)
struct NeLightData
{
	float position[3];
	uint32_t type;

	float direction[3];
	uint32_t __padding;

	float color[3];
	float intensity;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};
#pragma pack(pop)

struct NeCollectLights
{
	struct NeArray lightData;
};

#ifdef __cplusplus
}
#endif

#endif /* NE_SCENE_LIGHT_H */

/* NekoEngine
 *
 * Light.h
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
