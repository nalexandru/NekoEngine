#include "MTLDriver.h"

struct NeAccelerationStructure *
MTL_CreateAccelerationStructure(id<MTLDevice> dev, const struct NeAccelerationStructureCreateInfo *asci)
{
	struct NeAccelerationStructure *as = Sys_Alloc(sizeof(*as), 1, MH_RenderDriver);
	if (!as)
		return NULL;
	
	as->desc = [[MTLAccelerationStructureDescriptor alloc] init];
	as->desc.usage = MTLAccelerationStructureUsageNone;
	
	as->as = [dev newAccelerationStructureWithDescriptor: as->desc];
	
	return as;
}

uint64_t
MTL_AccelerationStructureHandle(id<MTLDevice> dev, const struct NeAccelerationStructure *as)
{
	return (uint64_t)as;
}

void
MTL_DestroyAccelerationStructure(id<MTLDevice> dev, struct NeAccelerationStructure *as)
{
	[as->as release];
	
	Sys_Free(as);
}
