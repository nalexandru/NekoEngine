#include "MTLBackend.h"

static id<MTLLibrary> _library;

void *
Re_ShaderModule(const char *name)
{
	@autoreleasepool {
		return [_library newFunctionWithName: [NSString stringWithUTF8String: name]];
	}
}

bool
MTL_InitLibrary(id<MTLDevice> dev)
{
	_library = [dev newDefaultLibrary];
	if (!_library)
		return false;
	
	return true;
}

void
MTL_TermLibrary(void)
{
	[_library release];
}
