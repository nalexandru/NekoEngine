/* NekoEngine
 *
 * mac_classes.m
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
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <AvailabilityMacros.h>

#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
	#import <QuartzCore/CAMetalLayer.h>
#endif

#include "mac_defs.h"

#include <string.h>
#include <stdbool.h>

#include <system/log.h>

#include <engine/input.h>
#include <engine/engine.h>
#include <engine/window.h>
#include <engine/keycodes.h>

#define MAC_IMPL_MODULE		"Platform_Mac"

extern ne_key cocoa_to_ne_keycodes[256];

@implementation NekoEngineApp

- (void)frame
{
	engine_frame();
}

@end

@implementation NekoEngineView

//@synthesize mouse_location;

- (void)keyUp:(NSEvent *)event
{
	input_key_event(cocoa_to_ne_keycodes[[event keyCode]], 0);
}

- (void)keyDown:(NSEvent *)event
{	
	log_entry(MAC_IMPL_MODULE, LOG_DEBUG, "keycode: 0x%x", [event keyCode]);
	input_key_event(cocoa_to_ne_keycodes[[event keyCode]], 1);
}

- (void)mouseUp:(NSEvent *)theEvent
{
	[super mouseUp:theEvent];
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[super mouseDown:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	[super mouseDragged:theEvent];
}

- (void)mouseMoved:(NSEvent *)evt
{
	[super mouseMoved:evt];
	mouse_location = NSMakePoint([evt absoluteX], [evt absoluteY]);
}

- (BOOL)canBecomeKeyView
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	log_entry(MAC_IMPL_MODULE, LOG_DEBUG, "accepts first responder");
	return YES;
}

- (BOOL)becomeFirstResponder
{
	log_entry(MAC_IMPL_MODULE, LOG_DEBUG, "become first responder");
	return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
	return YES;
}

- (void)viewDidEndLiveResize
{
	[super viewDidEndLiveResize];
	
	NSRect frame = [self frame];
	
	engine_screen_resized(frame.size.width,
						  frame.size.height);
}

#if defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_12
// Metal & Vulkan support

- (BOOL)wantsUpdateLayer
{
	return YES;
}

+ (Class)layerClass
{
	return [CAMetalLayer class];
}

- (CALayer *)makeBackingLayer
{
	CALayer *layer = [self.class.layerClass layer];
	CGSize vs = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
	layer.contentsScale = MIN(vs.width, vs.height);
	return layer;
}

#endif

@end

@implementation NekoEngineAppDelegate 

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    engine_destroy();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

- (void)applicationDidChangeOcclusionState:(NSNotification *)notification
{
	// pause when hidden ?
	// https://developer.apple.com/library/mac/documentation/Cocoa/Reference/ApplicationKit/Classes/NSApplication_Class/index.html#//apple_ref/c/data/NSApplicationDidChangeOcclusionStateNotification
}

/*- (void)windowDidResize:(NSNotification *)notification
{
	NSWindow *window = Platform::GetActiveWindow();
	CGSize size = window.contentView.frame.size;
	Engine::ScreenResized(size.width, size.height);
}*/

@end
