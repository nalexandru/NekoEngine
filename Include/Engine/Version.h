#ifndef _NE_ENGINE_VERSION_H_
#define _NE_ENGINE_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _NESTR
#define _NESTR_INTERNAL(x) #x
#define _NESTR(x) _NESTR_INTERNAL(x)
#endif

#define E_VER_MAJOR		0
#define E_VER_MINOR		8
#define E_VER_BUILD		282
#define E_VER_REVISION	0

#define E_PGM_NAME		"NekoEngine"
#define E_CODENAME		"Olivia"
#define E_CPY_STR		"2015-2023 Alexandru Naiman. All Rights Reserved."

#if E_VER_REVISION == 0
#	define E_VER_STR		_NESTR(E_VER_MAJOR) "." _NESTR(E_VER_MINOR) "." _NESTR(E_VER_BUILD)
#else
#	define E_VER_STR		_NESTR(E_VER_MAJOR) "." _NESTR(E_VER_MINOR) "." _NESTR(E_VER_BUILD) "." _NESTR(E_VER_REVISION)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_VERSION_H_ */

/* NekoEngine
 *
 * Version.h
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
