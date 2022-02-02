#include "NullGraphicsDriver.h"

struct NeAccelerationStructure *
NG_CreateAccelerationStructure(struct NeRenderDevice *dev, const struct NeAccelerationStructureCreateInfo *info)
{
	return NULL;
}

uint64_t
NG_AccelerationStructureHandle(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as)
{
	return 0;
}

void
NG_DestroyAccelerationStructure(struct NeRenderDevice *dev, struct NeAccelerationStructure *as)
{
}
