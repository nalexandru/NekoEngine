#ifndef _NE_ENGINE_BUILD_CONFIG_H_
#define _NE_ENGINE_BUILD_CONFIG_H_

#include <System/PlatformDetect.h>

/* Enable OpenXR support.
 * On Windows if using Visual Studio, the OpenXR SDK is installed from NuGet. For CMake the OpenXR SDK
 * (https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/) must be extracted in Deps/OpenXR
 * On *nix either build it yourself or install it from the package manager
 */
#ifndef ENABLE_OPENXR
#	define ENABLE_OPENXR	0
#endif

#if ENABLE_OPENXR && defined(SYS_PLATFORM_APPLE)
#	undef ENABLE_OPENXR
#	define ENABLE_OPENXR	0
#endif

#ifdef __cplusplus

#if __has_include(<steam/steam_api.h>)
#	include <steam/steam_api.h>
#	define ENABLE_STEAMWORKS
#endif

#endif

#endif /* _NE_ENGINE_BUILD_CONFIG_H_ */

/* NekoEngine
 *
 * BuildConfig.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
