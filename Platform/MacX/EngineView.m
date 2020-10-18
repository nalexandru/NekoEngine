//
//  EngineView.m
//  NekoEngine
//
//  Created by Alexandru Naiman on 9/28/20.
//  Copyright 2020 __MyCompanyName__. All rights reserved.
//

#define Handle __EngineHandle
#include <Input/Input.h>

#undef Handle
#import "EngineView.h"
#include "MacXPlatform.h"

@implementation EngineView

- (void)keyUp:(NSEvent *)e
{
	In_ButtonState[MacX_Keymap[[e keyCode]]] = false;
}

- (void)keyDown:(NSEvent *)e
{
	In_ButtonState[MacX_Keymap[[e keyCode]]] = true;
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

- (void)viewDidEndLiveResize
{
	[super viewDidEndLiveResize];
	
	//e_screen_resized([self frame].size.width, [self frame].size.height);
	
	[self setNeedsDisplay: YES];
}

@end
