#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define Handle __EngineHandle

#include <System/Window.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Render/Render.h>

#include "MacXPlatform.h"

#undef Handle

#import <Cocoa/Cocoa.h>
#import "EngineView.h"

int
Sys_CreateWindow(void)
{
#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
	NSUInteger mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
#else
	unsigned long mask = NSTitledWindowMask | NSClosableWindowMask |
		NSMiniaturizableWindowMask | NSResizableWindowMask;
#endif

	NSWindow *w = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(0, 0, *E_ScreenWidth, *E_ScreenHeight)
		styleMask: mask
		backing: NSBackingStoreBuffered
		defer: YES];
		
	[w cascadeTopLeftFromPoint: NSMakePoint(20, 20)];
	[w setTitle: @"NekoEngine"];

	NSView *v = (NSView *)[[EngineView alloc] initWithFrame: [(NSView *)[w contentView] frame]];

	[w setContentView: v];
	[w setInitialFirstResponder: v];
	[w makeFirstResponder: v];
	[w setAcceptsMouseMovedEvents: true];
	
	[w makeKeyAndOrderFront: NSApp];
	[w center];
	
	[NSApp activateIgnoringOtherApps: YES];

	E_Screen = (void *)w;

	return 0;
}

void
Sys_SetWindowTitle(const wchar_t *name)
{
	char *n = Sys_Alloc(wcslen(name), sizeof(*n) + 1, MH_Transient);
	wcstombs(n, name, wcslen(name));
	[(NSWindow *)E_Screen setTitle: [NSString stringWithFormat:@"%s", n]];
}

void
Sys_DestroyWindow(void)
{
}
