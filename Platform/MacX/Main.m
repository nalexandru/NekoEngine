//
//  main.m
//  NekoEngine
//
//  Created by Alexandru Naiman on 9/28/20.
//  Copyright __MyCompanyName__ 2020. All rights reserved.
//

#define Handle __EngineHandle

#include <Engine/Engine.h>
#include <System/System.h>

#undef Handle

#import <Cocoa/Cocoa.h>

int
main(int argc, char *argv[])
{
	int rc = -1;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if (!E_Init(argc, argv)) {
		NSAlert *a = [[NSAlert alloc] init];
		[a addButtonWithTitle:@"OK"];
		[a setMessageText:@"Failed to initialize engine. The program will" \
			" now exit."];
		[a setAlertStyle:NSCriticalAlertStyle];
		[a runModal];
		[a release];
		rc = -1;
	} else {
		rc = E_Run();
	}
	
	[pool release];
	return rc;
}
