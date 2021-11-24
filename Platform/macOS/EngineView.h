#import <Cocoa/Cocoa.h>


@interface EngineView : NSView {
	NSPoint mousePosition;
}

- (void)handleMouseMoved: (NSEvent *)e;

@end
