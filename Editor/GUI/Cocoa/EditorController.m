#include <Engine/Engine.h>

#import "EditorController.h"

@implementation EditorController

@synthesize assetManagerWindow;
@synthesize sceneHierarchyWindow;
@synthesize inspectorWindow;

- (id)init
{
	if (!(self = [super init]))
		return nil;

	return self;
}

#pragma mark Application Menu

- (void)showAboutPanel: (id)sender
{
	NSAlert *a = [[NSAlert alloc] init];
	[a addButtonWithTitle: @"OK"];
	[a setMessageText: @"About NekoEditor"];
	[a setInformativeText: @"NekoEditor v0.8 \"Olivia\"\nCopyright (c) 2015-2021 Alexandru Naiman. All rights reserved."];
	[a runModal];
	[a release];
}

- (void)showEditorPreferences: (id)sender
{
}

- (void)quit: (id)sender
{
	E_Shutdown();
}

#pragma mark File Menu

- (void)newProject: (id)sender
{
	NSSavePanel *savePanel = [NSSavePanel savePanel];
	[savePanel setCanCreateDirectories: YES];

	if ([savePanel runModal] != NSModalResponseOK)
		return;

	NSLog(@"NewProject: %@", [[savePanel URL] path]);
}

- (void)openProject: (id)sender
{
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	[openPanel setCanChooseFiles: YES];
	[openPanel setCanChooseDirectories: NO];
	[openPanel setAllowedFileTypes: [NSArray arrayWithObject: @"neproject"]];

	if ([openPanel runModal] != NSModalResponseOK)
		return;

	NSLog(@"OpenProject: %@", [[openPanel URL] path]);
}

- (void)saveProject: (id)sender
{
}

- (void)closeProject: (id)sender
{
}

#pragma mark Project Menu

- (void)showProjectSettings: (id)sender
{
}

#pragma mark Tools Menu

- (void)showSceneHierarchy: (id)sender
{
	[sceneHierarchyWindow makeKeyAndOrderFront: sender];
}

- (void)showAssetManager: (id)sender
{
	[assetManagerWindow makeKeyAndOrderFront: sender];
}

- (void)showInspector: (id)sender
{
	[inspectorWindow makeKeyAndOrderFront: sender];
}

#pragma mark Help Menu

- (void)showNativeReference: (id)sender
{
}

- (void)showScriptingReference: (id)sender
{
}

- (void)dealloc
{
	[super dealloc];
}

@end
