#define Handle __EngineHandle

#include <Engine/Engine.h>

#undef Handle

#import "EngineAppDelegate.h"

extern bool Darwin_screenVisible;

@implementation EngineAppDelegate

- (void)applicationWillTerminate: (NSNotification *)n
{
	E_Term();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)app
{
	return YES;
}

- (void)applicationDidChangeOcclusionState: (NSNotification *)n
{
	Darwin_screenVisible = [NSApp occlusionState] & NSApplicationOcclusionStateVisible;
}

@end
