#ifndef _NE_EDITOR_GUI_INSPECTOR_H_
#define _NE_EDITOR_GUI_INSPECTOR_H_

#import <Cocoa/Cocoa.h>

@interface Inspector : NSWindow
{
	NSTextField *_titleText;
}

- (id)initWithContentRect: (NSRect)contentRect;

- (void)inspectScene;
- (void)inspectEntity: (void *)entity;

@end

#endif
