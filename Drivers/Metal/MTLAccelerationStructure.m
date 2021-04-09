#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/AccelerationStructure.h>

#undef Handle

#include "MTLDriver.h"

struct AccelerationStructure *
MTL_CreateAccelerationStructure(id<MTLDevice> dev, const struct AccelerationStructureCreateInfo *asci)
{
	struct AccelerationStructure *as = calloc(1, sizeof(*as));
	if (!as)
		return NULL;
	
	as->desc = [[MTLAccelerationStructureDescriptor alloc] init];
	as->desc.usage = MTLAccelerationStructureUsageNone;
	
	as->as = [dev newAccelerationStructureWithDescriptor: as->desc];
	
	return as;
}

void
MTL_DestroyAccelerationStructure(id<MTLDevice> dev, struct AccelerationStructure *as)
{
	[as->as release];
	
	free(as);
}
