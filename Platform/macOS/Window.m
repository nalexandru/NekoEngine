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

@interface EngineWindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation EngineWindowDelegate

- (void)windowDidResize: (NSNotification *)notification
{
	NSWindow *w = (NSWindow *)E_screen;
	EngineView *v = (EngineView *)[w contentView];
	CAMetalLayer *metalLayer = (CAMetalLayer *)[v layer];
	
	const NSRect r = [v frame];
	[metalLayer setDrawableSize: r.size];
	
	E_ScreenResized(r.size.width, r.size.height);
}

- (void)windowWillClose: (NSNotification *)notification
{
	E_Shutdown();
}

@end

bool
Sys_CreateWindow(void)
{
	NSUInteger mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

	NSWindow *w = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(0, 0, *E_screenWidth, *E_screenHeight)
				  styleMask: mask
				    backing: NSBackingStoreBuffered
					  defer: YES];
		
	[w cascadeTopLeftFromPoint: NSMakePoint(20, 20)];
	[w setTitle: @"NekoEngine"];

	NSRect frame = [[w contentView] frame];
	NSView *v = (NSView *)[[EngineView alloc] initWithFrame: frame];

	[w setContentView: v];
	[w setInitialFirstResponder: v];
	[w makeFirstResponder: v];
	[w setAcceptsMouseMovedEvents: true];
	
	[w makeKeyAndOrderFront: NSApp];
	[w center];
	
	[w setDelegate: [[EngineWindowDelegate alloc] init]];
	
	[NSApp activateIgnoringOtherApps: YES];

	E_screen = (void *)w;
	*E_screenWidth = frame.size.width;
	*E_screenHeight = frame.size.height;
	
	return true;
}

void
Sys_SetWindowTitle(const char *name)
{
	[(NSWindow *)E_screen setTitle: [NSString stringWithUTF8String: name]];
}

void
Sys_DestroyWindow(void)
{
}
