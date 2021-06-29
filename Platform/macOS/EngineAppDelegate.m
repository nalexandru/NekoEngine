#define Handle __EngineHandle

#include <Engine/Engine.h>
#include <Engine/Application.h>

#undef Handle

#import "EngineAppDelegate.h"

extern bool Darwin_screenVisible;

@implementation EngineAppDelegate

- (void)about: (id)sender
{
	NSString *name = [[NSString alloc] initWithBytes: App_applicationInfo.name
											  length: wcslen(App_applicationInfo.name) * sizeof(wchar_t)
											encoding: NSUTF32LittleEndianStringEncoding];
	NSString *copyright = [[NSString alloc] initWithBytes: App_applicationInfo.copyright
													length: wcslen(App_applicationInfo.copyright) * sizeof(wchar_t)
												  encoding: NSUTF32LittleEndianStringEncoding];
	NSString *msg = [NSString stringWithFormat: @"Version: %d.%d.%d.%d\nCopyright (C) %@",
					 App_applicationInfo.version.major, App_applicationInfo.version.minor,
					 App_applicationInfo.version.build, App_applicationInfo.version.revision,
					 copyright];

	NSAlert *a = [[NSAlert alloc] init];
	[a addButtonWithTitle: @"OK"];
	[a setMessageText: name];
	[a setInformativeText: msg];
	[a runModal];
	[a release];
}

- (void)quit: (id)sender
{
	E_Shutdown();
}

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
