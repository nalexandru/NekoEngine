/* NekoEngine
 *
 * engine_mac.m
 * Author: Alexandru Naiman
 *
 * NekoEngine Mac (Cocoa) Main Loop
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

#define _DEFAULT_SOURCE

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include "mac_defs.h"

#include <stdint.h>
#include <stdbool.h>

#include <system/log.h>

#include <engine/input.h>
#include <engine/engine.h>
#include <engine/window.h>

extern bool engine_stop;
extern NSWindow *mac_active_window;
static bool _run = true;

int
sys_engine_run(void)
{
	NSEvent *e = NULL;

	[NSApp activateIgnoringOtherApps: YES];
	[mac_active_window makeKeyAndOrderFront: NSApp];

	while (!engine_stop) {
		while (1) {
			e = [NSApp nextEventMatchingMask: NSAnyEventMask
								   untilDate: [NSDate distantPast]
									  inMode: NSDefaultRunLoopMode
									 dequeue: true];
			if (!e)
				break;
			
			[NSApp sendEvent: e];
		}
		
		engine_frame();
	}
	
	return 0;
}
