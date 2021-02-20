#define Handle __EngineHandle
#include <Input/Input.h>
#undef Handle

#import "EngineView.h"

extern enum Button iOS_keymap[256];

@implementation EngineView

- (void)pressesBegan: (NSSet<UIPress *> *)presses withEvent: (UIPressesEvent *)event
{
	for (UIPress *p in presses) {
		if (!p.key)
			continue;
		
		In_buttonState[iOS_keymap[[p.key keyCode]]] = true;
	}
}

- (void)pressesEnded: (NSSet<UIPress *> *)presses withEvent: (UIPressesEvent *)event
{
	for (UIPress *p in presses) {
		if (!p.key)
			continue;
		
		In_buttonState[iOS_keymap[[p.key keyCode]]] = false;
	}
}

@end
