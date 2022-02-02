#include <Engine/IO.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>

#include "NullGraphicsDriver.h"

void *
NG_ShaderModule(struct NeRenderDevice *dev, const char *name)
{
	return dev;
}
