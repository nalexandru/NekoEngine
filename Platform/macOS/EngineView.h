#import <Cocoa/Cocoa.h>


@interface EngineView : NSView {
	NSPoint mousePosition;
	NSEventModifierFlags prevFlags;
}

- (void)handleMouseMoved: (NSEvent *)e;

@end
