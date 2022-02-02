#include <Input/Input.h>
#include <Engine/Engine.h>

#import "EngineView.h"
#include "macOSPlatform.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

extern bool __InSys_enableMouseAxis;

@implementation EngineView

- (void)handleMouseMoved: (NSEvent *)e
{
	if (!__InSys_enableMouseAxis)
		return;

	In_mouseAxis[0] = -([e deltaX] / ((float)*E_screenWidth / 2.f));
	In_mouseAxis[1] = -([e deltaY] / ((float)*E_screenHeight / 2.f));
}

+ (Class)layerClass
{
	return [CAMetalLayer class];
}

- (CALayer *)makeBackingLayer
{
	return [CAMetalLayer layer];
}

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
	NSEventModifierFlags flags = [e modifierFlags];

#define MODIFIER_KEY(x, y)								\
	if (((prevFlags & x) == x) && !((flags & x) == x))	\
		In_Key(y, false);								\
	if (!((prevFlags & x) == x) && ((flags & x) == x))	\
		In_Key(y, true)

	MODIFIER_KEY(NSEventModifierFlagCommand, BTN_KEY_LSUPER);
	MODIFIER_KEY(NSEventModifierFlagOption, BTN_KEY_LALT);
	MODIFIER_KEY(NSEventModifierFlagShift, BTN_KEY_LSHIFT);
	MODIFIER_KEY(NSEventModifierFlagControl, BTN_KEY_LCTRL);
	MODIFIER_KEY(NSEventModifierFlagCapsLock, BTN_KEY_CAPS);

	prevFlags = flags;
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

- (void)scrollWheel: (NSEvent *)e
{
	In_mouseAxis[2] = [e scrollingDeltaX];
}

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

@end
