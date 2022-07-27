#include <UI/UI.h>
#include <Math/Math.h>
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
	struct NeVec3 color;
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
	M_Vec3(&_activeRegion->color, r, g, b);

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

	Rt_ClearArray(&_regions, false);
}
