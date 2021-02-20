#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define Handle __EngineHandle

#include <System/Window.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
//#include <Render/Render.h>

//#include "MacXPlatform.h"

#undef Handle

#import <UIKit/UIKit.h>
#import "EngineViewController.h"

int
Sys_CreateWindow(void)
{
	CGRect r = [[UIScreen mainScreen] bounds];
	
	UIWindow *w = [[UIWindow alloc] initWithFrame: r];
	w.rootViewController = [[EngineViewController alloc] init];
	[w makeKeyAndVisible];
	[w retain];

	*E_screenWidth = r.size.width;
	*E_screenHeight = r.size.height;
	E_screen = (void *)w;

	return 0;
}

void
Sys_SetWindowTitle(const wchar_t *name)
{
	(void)name;
}

void
Sys_DestroyWindow(void)
{
}
