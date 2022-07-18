#include "MTLBackend.h"

struct NeAccelerationStructure *
Re_CreateAccelerationStructure(const struct NeAccelerationStructureCreateInfo *asci)
{
	struct NeAccelerationStructure *as = Sys_Alloc(sizeof(*as), 1, MH_RenderDriver);
	if (!as)
		return NULL;
	
	as->desc = [[MTLAccelerationStructureDescriptor alloc] init];
	as->desc.usage = MTLAccelerationStructureUsageNone;
	
	as->as = [MTL_device newAccelerationStructureWithDescriptor: as->desc];
	
	return as;
}

uint64_t
Re_AccelerationStructureHandle(const struct NeAccelerationStructure *as)
{
	return (uint64_t)as;
}

void
Re_DestroyAccelerationStructure(struct NeAccelerationStructure *as)
{
	[as->as release];
	
	Sys_Free(as);
}
