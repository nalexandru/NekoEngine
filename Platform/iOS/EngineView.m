#include <Input/Input.h>

#import "EngineView.h"

extern enum NeButton iOS_keymap[256];

@implementation EngineView

- (void)pressesBegan: (NSSet<UIPress *> *)presses withEvent: (UIPressesEvent *)event
{
	for (UIPress *p in presses) {
		if (!p.key)
			continue;
		
		In_Key(iOS_keymap[[p.key keyCode]], true);
	}
}

- (void)pressesEnded: (NSSet<UIPress *> *)presses withEvent: (UIPressesEvent *)event
{
	for (UIPress *p in presses) {
		if (!p.key)
			continue;
		
		In_Key(iOS_keymap[[p.key keyCode]], false);
	}
}

@end
