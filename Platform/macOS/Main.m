#define Handle __EngineHandle

#include <Engine/Engine.h>
#include <System/System.h>

#undef Handle

#import <Cocoa/Cocoa.h>

int
main(int argc, char *argv[])
{
	@autoreleasepool {
		if (!E_Init(argc, argv)) {
			NSAlert *a = [[NSAlert alloc] init];
			[a addButtonWithTitle: @"OK"];
			[a setMessageText: @"Failed to initialize engine. The program will now exit."];
			[a setAlertStyle:NSAlertStyleCritical];
			[a runModal];
			[a release];
		}
	}

	@autoreleasepool {
		return E_Run();
	}
}
