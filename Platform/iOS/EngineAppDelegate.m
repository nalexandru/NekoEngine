#include <System/System.h>
#include <Engine/Engine.h>

#import "EngineView.h"
#import "EngineAppDelegate.h"

@interface EngineAppDelegate ()

@end

extern bool Darwin_screenVisible;

@implementation EngineAppDelegate

- (BOOL)application: (UIApplication *)application didFinishLaunchingWithOptions: (NSDictionary *)launchOptions {
	int argc = 1;
	char *argv[4] =
	{
		"NekoEngine",
		NULL,
		NULL,
		NULL
	};
	
	bool rc = E_Init(argc, argv);
	if (!rc) {
		Sys_MessageBox(L"Fatal Error", L"Failed to initialize engine", MSG_ICON_ERROR);
		return false;
	}
	
	[(EngineView *)[[(UIWindow *)E_screen rootViewController] view] setDelegate: self];
	
	return true;
}

- (void)applicationWillTerminate: (UIApplication *)application
{
	E_Term();
}

- (void)applicationDidEnterBackground: (UIApplication *)application
{
	Darwin_screenVisible = false;
}

- (void)applicationDidBecomeActive: (UIApplication *)application
{
	Darwin_screenVisible = true;
}

- (void)applicationDidReceiveMemoryWarning: (UIApplication *)application
{
	
}

- (void)mtkView: (MTKView *)view drawableSizeWillChange: (CGSize)size
{
	// resize
}

- (void)drawInMTKView: (MTKView *)view
{
	E_Frame();
}

@end
