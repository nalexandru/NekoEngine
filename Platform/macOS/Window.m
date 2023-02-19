#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <System/Window.h>
#include <System/Memory.h>
#include <Engine/Engine.h>

#include "macOSPlatform.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#import "EngineView.h"

#include <System/Log.h>

static NSWindow *_wnd = nil;

@interface EngineWindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation EngineWindowDelegate

- (void)windowDidResize: (NSNotification *)notification
{
	
}

- (void)windowWillClose: (NSNotification *)notification
{
	E_Shutdown();
}

@end

bool
Sys_CreateWindow(void)
{
	if (_wnd)
		return true;
	
	NSUInteger mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

	_wnd = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(0, 0, *E_screenWidth, *E_screenHeight)
				  styleMask: mask
				    backing: NSBackingStoreBuffered
					  defer: YES];
		
	[_wnd cascadeTopLeftFromPoint: NSMakePoint(20, 20)];
	[_wnd setTitle: @"NekoEngine"];

	NSRect frame = [[_wnd contentView] frame];
	NSView *v = (NSView *)[[EngineView alloc] initWithFrame: frame];

	[_wnd setContentView: v];
	[_wnd setInitialFirstResponder: v];
	[_wnd makeFirstResponder: v];
	[_wnd setAcceptsMouseMovedEvents: true];
	
	[_wnd makeKeyAndOrderFront: NSApp];
	[_wnd center];
	
	[_wnd setDelegate: [[EngineWindowDelegate alloc] init]];
	
	[NSApp activateIgnoringOtherApps: YES];

	E_screen = (void *)v;
	*E_screenWidth = frame.size.width;
	*E_screenHeight = frame.size.height;
	
	return true;
}

void
Sys_SetEngineWindow(void *window)
{
	NSView *view = (NSView *)window;
	const NSRect r = [view frame];
	
	*E_screenWidth = r.size.width;
	*E_screenHeight = r.size.height;
	
	E_screen = window;
	_wnd = [view window];
}

void
Sys_SetWindowTitle(const char *name)
{
	[_wnd setTitle: [NSString stringWithUTF8String: name]];
}

void
Sys_MoveWindow(int x, int y)
{
	/*RECT rc;
	GetWindowRect((HWND)E_screen, &rc);
	MoveWindow((HWND)E_screen, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE);*/
}

void
Sys_ShowWindow(bool show)
{
	if (show)
		[_wnd orderFront: nil];
	else
		[_wnd orderBack: nil];
}

void
Sys_WorkArea(int *top, int *left, int *right, int *bottom)
{
	NSRect bounds = [[NSScreen mainScreen] visibleFrame];
	
	if (top) *top = bounds.origin.y;
	if (left) *left = bounds.origin.x;
	if (right) *right = bounds.size.width;
	if (bottom) *bottom = bounds.size.height;
}

void
Sys_NonClientMetrics(int32_t *titleBarHeight, int32_t *borderHeight, int32_t *borderWidth)
{
	if (titleBarHeight) *titleBarHeight = 0;
	if (borderHeight) *borderHeight = 0;
	if (borderWidth) *borderWidth = 0;
}

void
Sys_DestroyWindow(void)
{
}

/* NekoEngine
 *
 * Window.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
