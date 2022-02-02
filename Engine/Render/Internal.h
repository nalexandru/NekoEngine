#ifndef _NE_RENDER_INTERNAL_H_
#define _NE_RENDER_INTERNAL_H_

#include <Render/Types.h>
#include <Runtime/Runtime.h>

struct NeRenderGraph
{
	struct NeArray execPasses;
	struct NeArray resources;
	struct NeArray allPasses;
	struct NeSemaphore *semaphore;
};

#endif /* _NE_RENDER_INTERNAL_H_ */