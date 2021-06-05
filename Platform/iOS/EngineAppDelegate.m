#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>

#include <pthread.h>

#import "EngineView.h"
#import "EngineAppDelegate.h"

bool Sys_InitDarwinPlatform(void);

@interface EngineAppDelegate ()

@property (atomic) BOOL downloadCompleted;

@end

extern bool Darwin_screenVisible;
extern NSURL *Darwin_appSupportURL;

@implementation EngineAppDelegate

@synthesize downloadCompleted;

- (void)downloadAssets
{
	NSURL *dest = [[NSURL alloc] initFileURLWithPath: @"GameData.zip" relativeToURL: Darwin_appSupportURL];
	
	if ([[NSFileManager defaultManager] fileExistsAtPath: [dest path]])
	//	[[NSFileManager defaultManager] removeItemAtPath: [dest path] error: nil];
		return;
	
	NSString *plistFile = [[NSBundle mainBundle] pathForResource: @"Downloader" ofType: @"plist"];
	if (!plistFile)
		return;
	
	NSDictionary *config = [[NSDictionary alloc] initWithContentsOfFile: plistFile];
	
	__weak typeof(self) weakSelf = self;
	NSString *url = [config objectForKey: @"AssetURL"];
	NSURLSessionDownloadTask *task = [[NSURLSession sharedSession] downloadTaskWithURL: [NSURL URLWithString: url]
									completionHandler:^(NSURL * _Nullable location, NSURLResponse * _Nullable response, NSError * _Nullable error) {
		
		char buff[4096];
		Sys_DirectoryPath(SD_APP_DATA, buff, sizeof(buff));
		
		[[NSFileManager defaultManager] moveItemAtURL: location toURL: dest error: nil];
		
		[weakSelf setDownloadCompleted: true];
	}];
	
	[self setDownloadCompleted: false];
	[task resume];
	
	UIAlertController *ctl =[UIAlertController alertControllerWithTitle: @"NekoEngine"
																message: @"Downloading Assets"
														 preferredStyle: UIAlertControllerStyleAlert];
	
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[[(UIWindow *)E_screen rootViewController] presentViewController: ctl animated: true completion: nil];
	});
	
	while (![self downloadCompleted])
		sched_yield();
	
	dispatch_async(dispatch_get_main_queue(), ^(void){
		[ctl dismissViewControllerAnimated: true completion: nil];
	});
}

- (BOOL)startEngine
{
	NSString *dataDir = [[NSString alloc] initWithString: [Darwin_appSupportURL path]];
	
	int argc = 3;
	char *argv[4] =
	{
		"NekoEngine",
		"-d",
		(char *)[[dataDir stringByAppendingPathComponent:@"Data"] UTF8String],
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

- (BOOL)application: (UIApplication *)application didFinishLaunchingWithOptions: (NSDictionary *)launchOptions
{
	Sys_InitDarwinPlatform();
	
	__weak typeof(self) weakSelf = self;
	dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INTERACTIVE, 0), ^{
		[weakSelf downloadAssets];
		
		dispatch_async(dispatch_get_main_queue(), ^(void){
			if (![weakSelf startEngine])
				exit(EXIT_FAILURE);
		});
	});
	
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
	E_ScreenResized(size.width, size.height);
}

- (void)drawInMTKView: (MTKView *)view
{
	E_Frame();
	Sys_ResetHeap(MH_Transient);
}

@end
