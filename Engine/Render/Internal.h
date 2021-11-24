#ifndef _NE_RENDER_INTERNAL_H_
#define _NE_RENDER_INTERNAL_H_

#include <Render/Types.h>
#include <Runtime/Runtime.h>

struct RenderGraph
{
	struct Array execPasses;
	struct Array resources;
	struct Array allPasses;
	struct Semaphore *semaphore;
};

#endif /* _NE_RENDER_INTERNAL_H_ */