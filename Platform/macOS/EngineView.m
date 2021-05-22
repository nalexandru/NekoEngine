#define Handle __EngineHandle
#include <Input/Input.h>
#undef Handle

#import "EngineView.h"
#include "macOSPlatform.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@implementation EngineView

+ (Class)layerClass
{
	return [CAMetalLayer class];
}

- (CALayer *)makeBackingLayer
{
	return [CAMetalLayer layer];
}

- (void)keyUp:(NSEvent *)e
{
	In_buttonState[macOS_keymap[[e keyCode]]] = false;
}

- (void)keyDown:(NSEvent *)e
{
	In_buttonState[macOS_keymap[[e keyCode]]] = true;
}

- (void)mouseUp:(NSEvent *)e
{
}

- (void)mouseDown:(NSEvent *)e
{
}

- (void)mouseDragged:(NSEvent *)e
{
}

- (void)mouseMoved:(NSEvent *)e
{
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

- (BOOL)acceptsFirstMouse:(NSEvent *)e
{
	return YES;
}

@end
