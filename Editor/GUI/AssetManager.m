#define Handle __EngineHandle

#include <Engine/Engine.h>

#undef Handle

#import "AssetManager.h"

@interface AssetManagerWindowDelegate : NSObject<NSWindowDelegate>
@end

@implementation AssetManagerWindowDelegate

- (BOOL)windowShouldClose: (id)sender
{
	return YES;
}

- (void)windowWillClose: (NSNotification *)notification
{
	E_Shutdown();
}

@end

@implementation AssetManager

- (id)initWithContentRect: (NSRect)contentRect;
{
	if (!(self = [super initWithContentRect: contentRect
								  styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
									backing: NSBackingStoreBuffered
									  defer: YES]))
		return nil;
	
	[self setDelegate: [[AssetManagerWindowDelegate alloc] init]];

	NSView *view = [self contentView];

	NSButton *btn = [[NSButton alloc] initWithFrame: NSMakeRect(5, contentRect.size.height - 30, 75, 25)];
	[btn setTitle: @"Import"];
	[btn setTarget: self];
	[btn setAction: @selector(importAsset:)];
#ifdef __APPLE__
	[btn setBezelStyle: NSBezelStyleRounded];
#endif
	[view addSubview: btn];

	NSScrollView *scrollView = [[[NSScrollView alloc] initWithFrame: NSMakeRect(5, 5, contentRect.size.width - 10, contentRect.size.height - 40)] autorelease];
	[scrollView setHasHorizontalScroller: YES];
	[scrollView setHasVerticalScroller: YES];
	[scrollView setBorderType: NSBezelBorder];

	_assetView = [[NSGridView alloc] initWithFrame: NSMakeRect(5, 5, contentRect.size.width - 10, contentRect.size.height - 40)];
	[scrollView setDocumentView: _assetView];

	[view addSubview: scrollView];

	return self;
}

- (void)importAsset: (id)sender
{
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	[openPanel setCanChooseFiles: YES];
	[openPanel setCanChooseDirectories: NO];

	if ([openPanel runModal] != NSModalResponseOK)
		return;

	NSLog(@"ImportAsset: %@", [[openPanel URL] path]);
}

- (void)dealloc
{
	[super dealloc];
}

@end
