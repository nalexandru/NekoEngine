#define Handle __EngineHandle

#include <Render/Device.h>

#undef Handle

#include "MTLDriver.h"

static id<MTLLibrary> _library;

bool
MTL_InitLibrary(id<MTLDevice> dev)
{
	_library = [dev newDefaultLibrary];
	if (!_library)
		return false;
	
	return true;
}

id<MTLFunction>
MTL_ShaderModule(id<MTLDevice> dev, const char *name)
{
	@autoreleasepool {
		return [_library newFunctionWithName: [NSString stringWithUTF8String: name]];
	}
}

void
MTL_TermLibrary(void)
{
	[_library release];
}
