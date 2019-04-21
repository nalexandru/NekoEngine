/* NekoEngine
 *
 * window_mac.m
 * Author: Alexandru Naiman
 *
 * NekoEngine Mac (Cocoa) Window Subsystem
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

#import <Cocoa/Cocoa.h>
#include <AvailabilityMacros.h>
#include "mac_defs.h"

#include <string.h>
#include <stdbool.h>

#include <system/log.h>

#include <engine/input.h>
#include <engine/window.h>
#include <engine/keycodes.h>

#define WINDOW_UNIX_MODULE		"Window_Cocoa"

NSView *mac_ns_view = NULL;
NekoEngineView *mac_view = NULL;
NekoEngineAppDelegate *mac_app_delegate = NULL;

NSWindow *mac_active_window = NULL;
uint16_t mac_wnd_width = 0, mac_wnd_height = 0;
static bool _fullscreen = false, _own_window = false;

ne_status
window_create(
	char *title,
	uint16_t w,
	uint16_t h)
{
	NSApplicationLoad();
	[NSApplication sharedApplication];
	[NSApp finishLaunching];
	
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
#endif
	
	mac_app_delegate = [[NekoEngineAppDelegate alloc] init];
	[NSApp setDelegate: mac_app_delegate];
	[[NSApplication sharedApplication] setDelegate: mac_app_delegate];
	
	NSMenu *menu_bar = [[NSMenu alloc] init];
	NSMenuItem *app_item = [[NSMenuItem alloc] init];
	
	[app_item setTitle:[NSString stringWithUTF8String: title]];
	
	[menu_bar addItem:app_item];
	[NSApp setMainMenu:menu_bar];
	
	NSMenu *app_menu = [[NSMenu alloc] init];
	NSMenuItem *quit_item = [[NSMenuItem alloc] initWithTitle:@"Quit NekoEngine" action:@selector(terminate:) keyEquivalent:@"q"];
	[app_menu addItem:quit_item];
	[app_item setSubmenu:app_menu];
	
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
	NSUInteger s_mask = NSWindowStyleMaskTitled |
		NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable |
		NSWindowStyleMaskResizable;
#else
	unsigned long s_mask = NSTitledWindowMask |
		NSClosableWindowMask |
		NSMiniaturizableWindowMask |
		NSResizableWindowMask;
#endif
	
	mac_active_window = [[NSWindow alloc]
						 initWithContentRect: NSMakeRect(0, 0, w, h)
						 styleMask: s_mask
						 backing: NSBackingStoreBuffered
						 defer: YES];
	
	[mac_active_window cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
	[mac_active_window setTitle:[NSString stringWithUTF8String: title]];
	
	mac_view = [[NekoEngineView alloc]
				initWithFrame:[(NSView *)[mac_active_window contentView] frame]];
	
	[mac_active_window setContentView: mac_view];
	[mac_active_window setInitialFirstResponder: mac_view];
	[mac_active_window makeFirstResponder: mac_view];
	[mac_active_window setAcceptsMouseMovedEvents: true];
	
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
	mac_view.wantsLayer = YES;
#endif
	
	mac_ns_view = (NSView *)mac_view;
	
	_own_window = true;
	
	[mac_active_window makeKeyAndOrderFront: NSApp];
	[mac_active_window center];
	
	return NE_OK;
}

ne_status
window_register(void *handle)
{
	mac_active_window = handle;

	return NE_OK;
}

ne_status
window_resize(uint16_t w, uint16_t h)
{
	return NE_FAIL;
}

ne_status
window_set_title(char *title)
{
	[mac_active_window setTitle:[NSString stringWithUTF8String:title]];
	return NE_OK;
}

ne_status
window_fullscreen(uint16_t w, uint16_t h)
{
	return NE_FAIL;
}

void
window_destroy(void)
{
	//
}
