#ifndef NE_ENGINE_APPLICATION_H
#define NE_ENGINE_APPLICATION_H

#include <Engine/Types.h>
#include <System/PlatformDetect.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeApplicationInfo
{
	const char name[64];
	const char copyright[64];
	struct NeVersion version;
};

extern struct NeApplicationInfo App_applicationInfo;

bool App_EarlyInit(int argc, char *argv[]);
bool App_InitApplication(int argc, char *argv[]);
void App_Frame(void);
void App_TermApplication(void);

#ifdef __cplusplus
}
#endif

#define NE_APPLICATION(n, c, maj, min, build, rev)	\
struct NeApplicationInfo App_applicationInfo =	\
{												\
	n,											\
	c,											\
	{ maj, min, build, rev }					\
}

#endif /* NE_ENGINE_APPLICATION_H */

/* NekoEngine
 *
 * Application.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
