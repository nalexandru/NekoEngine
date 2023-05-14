#include <Math/Math.h>
#include <Scene/Light.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>

static bool InitLight(struct NeLight *l, const char **args);
static void TermLight(struct NeLight *l);

NE_REGISTER_COMPONENT(NE_LIGHT, struct NeLight, 16, InitLight, nullptr, TermLight)

static bool
InitLight(struct NeLight *l, const char **args)
{
	l->type = LT_Directional;
	l->color.x = l->color.y = l->color.z = 1.f;
	l->intensity = 1.f;

	l->innerRadius = l->outerRadius = l->innerCutoff = l->outerCutoff = 0.f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		const size_t len = strlen(arg);

		if (!strncmp(arg, "Type", len)) {
			const char *val = *(++args);
			const size_t vLen = strlen(val);

			if (!strncmp(val, "Directional", vLen))
				l->type = LT_Directional;
			else if (!strncmp(val, "Point", vLen))
				l->type = LT_Point;
			else if (!strncmp(val, "Spot", vLen))
				l->type = LT_Spot;
		} else if (!strncmp(arg, "Color", len)) {
			char *ptr = (char *)*(++args);
			l->color.x = strtof(ptr, &ptr);
			l->color.y = strtof(ptr + 2, &ptr);
			l->color.z = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "Intensity", len)) {
			l->intensity = (float)atof(*(++args));
		} else if (!strncmp(arg, "InnerRadius", len)) {
			l->innerRadius = (float)atof(*(++args));
		} else if (!strncmp(arg, "OuterRadius", len)) {
			l->outerRadius = (float)atof(*(++args));
		} else if (!strncmp(arg, "InnerCutoff", len)) {
			l->innerCutoff = cosf(XMConvertToRadians((float)atof(*(++args))));
		} else if (!strncmp(arg, "OuterCutoff", len)) {
			l->outerCutoff = cosf(XMConvertToRadians((float)atof(*(++args))));
		}
	}

	return true;
}

NE_SYSTEM(SCN_COLLECT_LIGHTS, ECSYS_GROUP_MANUAL, 0, true, struct NeCollectLights, 2, NE_TRANSFORM, NE_LIGHT)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeLight *l = (struct NeLight *)comp[1];

	const float x = XMConvertToRadians(M_QuatRoll(&xform->rotation));
	const float y = XMConvertToRadians(M_QuatPitch(&xform->rotation));

	const float cosy = cosf(y);

	const float yPos = l->type != LT_Directional ? xform->position.y : -xform->position.y;

	struct NeLightData ld =
	{
		.position = { xform->position.x, yPos, xform->position.z },
		.type = (uint32_t)l->type,
		.direction =
		{
			sinf(x) * cosy,
			sinf(y),
			cosf(x) * cosy 
		},
		.color = { l->color.x, l->color.y, l->color.z },
		.intensity = l->intensity,
		.innerRadius = l->innerRadius,
		.outerRadius = l->outerRadius,
		.innerCutoff = l->innerCutoff,
		.outerCutoff = l->outerCutoff
	};

	Rt_ArrayAdd(&args->lightData, &ld);
}

static void
TermLight(struct NeLight *l)
{
}

/* NekoEngine
 *
 * Light.c
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
