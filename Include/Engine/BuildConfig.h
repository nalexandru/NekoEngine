#ifndef _NE_ENGINE_BUILD_CONFIG_H_
#define _NE_ENGINE_BUILD_CONFIG_H_

#include <System/PlatformDetect.h>

/* Enable OpenXR support.
 * On Windows the OpenXR SDK (https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/) must be extracted in Deps/OpenXR
 * On *nix either build it yourself or install it from the package manager
 */
#ifndef ENABLE_OPENXR
#	define ENABLE_OPENXR	0
#endif

#if ENABLE_OPENXR && defined(SYS_PLATFORM_APPLE)
#	undef ENABLE_OPENXR
#	define ENABLE_OPENXR	0
#endif

#endif /* _NE_ENGINE_BUILD_CONFIG_H_ */
