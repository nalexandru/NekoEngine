#include <Input/Input.h>
#include <Engine/Engine.h>

#import "EngineView.h"
#include "macOSPlatform.h"

#ifdef NE_MACOS_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

extern bool In_p_rawMouseAxis;

@implementation EngineView

- (void)handleMouseMoved: (NSEvent *)e
{
	if (!In_p_rawMouseAxis || !In_pointerCaptured)
		return;

	In_mouseAxis[0] = (float)(-([e deltaX] / ((float)*E_screenWidth / 2.f)));
	In_mouseAxis[1] = (float)(-([e deltaY] / ((float)*E_screenHeight / 2.f)));
}

#ifdef NE_MACOS_METAL
+ (Class)layerClass
{
	return [CAMetalLayer class];
}

- (CALayer *)makeBackingLayer
{
	return [CAMetalLayer layer];
}
#endif

- (void)keyUp: (NSEvent *)e
{
	In_Key(macOS_keymap[[e keyCode]], false);
}

- (void)keyDown: (NSEvent *)e
{
	In_Key(macOS_keymap[[e keyCode]], true);
}

- (void)flagsChanged: (NSEvent *)e
{
	const NSUInteger flags = [e modifierFlags];

#define MODIFIER_KEY(x, y)										\
	if (((prevFlags & (x)) == (x)) && !((flags & (x)) == (x)))	\
		In_Key((y), false);										\
	if (!((prevFlags & (x)) == (x)) && ((flags & (x)) == (x)))	\
		In_Key((y), true)

	MODIFIER_KEY(NSEventModifierFlagCommand, BTN_KEY_LSUPER);
	MODIFIER_KEY(NSEventModifierFlagOption, BTN_KEY_LALT);
	MODIFIER_KEY(NSEventModifierFlagShift, BTN_KEY_LSHIFT);
	MODIFIER_KEY(NSEventModifierFlagControl, BTN_KEY_LCTRL);
	MODIFIER_KEY(NSEventModifierFlagCapsLock, BTN_KEY_CAPS);

	prevFlags = flags;
#undef MODIFIER_KEY
}

- (void)mouseUp: (NSEvent *)e
{
	In_buttonState[BTN_MOUSE_LMB] = false;
}

- (void)mouseDown: (NSEvent *)e
{
	In_buttonState[BTN_MOUSE_LMB] = true;
}

- (void)rightMouseUp: (NSEvent *)e
{
	In_buttonState[BTN_MOUSE_RMB] = false;
}

- (void)rightMouseDown: (NSEvent *)e
{
	In_buttonState[BTN_MOUSE_RMB] = true;
}

- (void)otherMouseUp: (NSEvent *)e
{
	In_buttonState[BTN_MOUSE_LMB + [e buttonNumber]] = false;
}

- (void)otherMouseDown: (NSEvent *)e
{
	In_buttonState[BTN_MOUSE_LMB + [e buttonNumber]] = true;
}

#if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
- (void)scrollWheel: (NSEvent *)e
{
	In_mouseAxis[2] = (float)[e scrollingDeltaX];
}
#endif

- (void)mouseDragged: (NSEvent *)e
{
	[super mouseDragged: e];
	[self handleMouseMoved: e];
}

- (void)rightMouseDragged: (NSEvent *)e
{
	[super rightMouseDragged: e];
	[self handleMouseMoved: e];
}

- (void)otherMouseDragged: (NSEvent *)e
{
	[super otherMouseDragged: e];
	[self handleMouseMoved: e];
}

- (void)mouseMoved: (NSEvent *)e
{
	[super mouseMoved: e];
	[self handleMouseMoved: e];
}

- (BOOL)canBecomeKeyView
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)becomeFirstResponder
{
	return YES;
}

- (BOOL)acceptsFirstMouse: (NSEvent *)e
{
	return YES;
}

- (void)viewDidEndLiveResize
{
	[super viewDidEndLiveResize];

#ifdef NE_MACOS_METAL
	CAMetalLayer *metalLayer = (CAMetalLayer *)[self layer];
	[metalLayer setDrawableSize: self.frame.size];
#endif

	E_ScreenResized(self.frame.size.width, self.frame.size.height);
}

- (void)setFrameSize: (NSSize)newSize
{
	[super setFrameSize: newSize];

	if ([self inLiveResize])
		return;

#ifdef NE_MACOS_METAL
	CAMetalLayer *metalLayer = (CAMetalLayer *)[self layer];
	[metalLayer setDrawableSize: self.frame.size];
#endif

	E_ScreenResized(self.frame.size.width, self.frame.size.height);
}

@end

/* NekoEngine
 *
 * EngineView.m
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
