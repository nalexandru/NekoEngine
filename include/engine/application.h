/* NekoEngine
 *
 * application.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Application Interface
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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

#ifndef _NE_APPLICATION_H_
#define _NE_APPLICATION_H_

#include <stdint.h>
#include <stdbool.h>

#include <engine/status.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NE_APP_API_VER		2

typedef struct ne_app_module
{
	uint32_t api_ver;
	ne_status (*init)(void);
	void (*update)(double);
	void (*draw)(void);
	void (*screen_resized)(uint16_t, uint16_t);
	void (*release)(void);
} ne_app_module;

typedef const ne_app_module *(*create_app_module_proc)(void);
/*
 * Application modules must implement C linked function
 * returning a ne_app_module struct called
 * create_app_module. The returned pointer must
 * remain valid for the application lifetime.
 *
 * const ne_app_module *create_app_module(void);
 *
 * On Win32 this function must be exported
 */

#ifdef _NE_ENGINE_INTERNAL_

extern const ne_app_module *app_module;

ne_status	app_init(void);
void		app_release(void);

static inline void
app_update(double dt)
{
	if (app_module)
		app_module->update(dt);
}

static inline void
app_draw(void)
{
	if (app_module)
		app_module->draw();
}

static inline void
app_screen_resized(uint16_t width,
	uint16_t height)
{
	if (app_module)
		app_module->screen_resized(width, height);
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* _NE_APPLICATION_H_ */

