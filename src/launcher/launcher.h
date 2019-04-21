/* NekoEngine
 *
 * launcher.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Launcher
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2018, Alexandru Naiman
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
 */

#ifndef _NE_LAUNCHER_H_
#define _NE_LAUNCHER_H_

#define LAUNCHER_WIDTH		350
#define LAUNCHER_HEIGHT		400
#define LAUNCHER_TITLE		"NekoEngine Launcher"
#define BANNER_HEIGHT		120
#define BANNER_WIDTH		330

#ifdef _LAUNCHER_IMPLEMENTATION_
#include <runtime/runtime.h>

static char *_launcher_res_str[16] =
{
	"640x480",
	"800x600",
	"1024x600",
	"1024x768",
	"1152x864",
	"1280x720",
	"1280x800",
	"1280x1024",
	"1366x768",
	"1440x900",
	"1600x900",
	"1680x1050",
	"1920x1080",
	"1920x1200",
	"2560x1440",
	"3840x2160"
};
#define _launcher_res_count() 16
#endif

#endif /* _NE_LAUNCHER_H_ */
