#include <System/System.h>
#include <System/Memory.h>
#include <Engine/Engine.h>

#include <pthread.h>

#import "EngineView.h"
#import "EngineAppDelegate.h"

bool Sys_InitDarwinPlatform(void);

@interface EngineAppDelegate ()

@property (atomic) BOOL downloadCompleted;
@property (atomic) BOOL downloadRequestFinished;

@end

extern bool Darwin_screenVisible;
extern NSURL *Darwin_appSupportURL;

@implementation EngineAppDelegate

@synthesize downloadCompleted;
@synthesize downloadRequestFinished;

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

	UIAlertController *ctl = [UIAlertController alertControllerWithTitle: @"NekoEngine"
																 message: @"Downloading Assets"
														  preferredStyle: UIAlertControllerStyleAlert];

	dispatch_async(dispatch_get_main_queue(), ^(void){
		[[(UIWindow *)E_screen rootViewController] presentViewController: ctl animated: true completion: nil];
	});

	[self setDownloadCompleted: false];
	[self setDownloadRequestFinished: false];

	__weak typeof(self) weakSelf = self;
	NSString *url = [config objectForKey: @"AssetURL"];
	while (![self downloadCompleted]) {
		NSURLSessionDownloadTask *task = [[NSURLSession sharedSession] downloadTaskWithURL: [NSURL URLWithString: url]
																		 completionHandler:^(NSURL * _Nullable location, NSURLResponse * _Nullable response, NSError * _Nullable error) {
			if (error) {
				NSLog(@"Download error: %@", error);
			} else {
				[[NSFileManager defaultManager] moveItemAtURL: location toURL: dest error: nil];
				[weakSelf setDownloadCompleted: true];
			}

			[weakSelf setDownloadRequestFinished: true];
		}];

	
		[self setDownloadCompleted: false];
		[self setDownloadRequestFinished: false];
		[task resume];

		while (![self downloadRequestFinished])
			sched_yield();
	}
	
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
		Sys_MessageBox("Fatal Error", "Failed to initialize engine", MSG_ICON_ERROR);
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

/* NekoEngine
 *
 * EngineAppDelegate.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
