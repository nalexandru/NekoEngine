#include <UI/UI.h>
#include <Engine/Engine.h>
#include <Engine/Profiler.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>
#include <Scene/Components.h>

#define PROFILER_DRAW_SYS	"ProfilerDraw"

struct Marker
{
	const char *name;
	double time;
};

struct Region
{
	const char *name;
	double start;
	double end;
	float color[3];
	struct NeArray markers;
};

struct ProfilerDraw
{
	NE_COMPONENT_BASE;
};

static bool _InitDraw(struct ProfilerDraw *xform, const void **args) { return true; }
static void _TermDraw(struct ProfilerDraw *xform) { }

E_REGISTER_COMPONENT(PROFILER_DRAW_COMP, struct ProfilerDraw, 16, _InitDraw, _TermDraw)

static struct NeArray _regions = { .elemSize = sizeof(struct Region), .align = 1, .heap = MH_System };
static struct Region *_activeRegion;

void
Prof_BeginRegion(const char *name, float r, float g, float b)
{
	_activeRegion = Rt_ArrayAllocate(&_regions);

	_activeRegion->name = name;
	_activeRegion->start = E_Time();
	_activeRegion->color[0] = r;
	_activeRegion->color[1] = g;
	_activeRegion->color[2] = b;

	Rt_InitArray(&_activeRegion->markers, 200, sizeof(struct Marker), MH_Transient);
}

void
Prof_InsertMarker(const char *name)
{
	if (!_activeRegion)
		return;

	struct Marker *m = Rt_ArrayAllocate(&_activeRegion->markers);
	m->name = name;
	m->time = E_Time();
}

void
Prof_EndRegion(void)
{
	_activeRegion->end = E_Time();
	_activeRegion = NULL;
}

void
Prof_Reset(void)
{
	Rt_ClearArray(&_regions, false);
}

E_SYSTEM(PROFILER_DRAW_SYS, ECSYS_GROUP_POST_LOGIC, 0, false, void, 2, PROFILER_DRAW_COMP, UI_CONTEXT_COMP)
{
	//struct ProfilerDraw *pd = comp[0];
	struct NeUIContext *ctx = comp[1];

	char buff[256];

	float y = 100.f;

	struct Region *r;
	Rt_ArrayForEach(r, &_regions) {
		snprintf(buff, sizeof(buff), "%s: %.02f ms", r->name, (r->end - r->start) * 1000.f);
		
		UI_DrawText(ctx, buff, 0.f, y, 20.f, NULL);
		y += 20.f;

		double prevTime = r->start;
		for (size_t i = 0; i < r->markers.count; ++i) {
			struct Marker *m = Rt_ArrayGet(&r->markers, i);

			snprintf(buff, sizeof(buff), "%s: %.02f ms", m->name, (m->time - prevTime) * 1000.f);
			UI_DrawText(ctx, buff, 20.f, y, 20.f, NULL);
			y += 20.f;

			prevTime = m->time;
		}
	}
}

/* NekoEngine
 *
 * Profiler.c
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
