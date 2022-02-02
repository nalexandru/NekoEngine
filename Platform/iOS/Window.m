#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <System/Window.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
//#include <Render/Render.h>

//#include "MacXPlatform.h"

#import <UIKit/UIKit.h>

#import "EngineView.h"
#import "EngineViewController.h"

bool
Sys_CreateWindow(void)
{
	CGRect r = [[UIScreen mainScreen] bounds];
	
	UIWindow *w = [[UIWindow alloc] initWithFrame: r];
	w.rootViewController = [[EngineViewController alloc] init];
	[w makeKeyAndVisible];
	[w retain];
	
	CGSize size = [(EngineView *)[[w rootViewController] view] drawableSize];

	*E_screenWidth = size.width;
	*E_screenHeight = size.height;
	E_screen = (void *)w;

	return true;
}

void
Sys_SetWindowTitle(const char *name)
{
	(void)name;
}

void
Sys_DestroyWindow(void)
{
}
