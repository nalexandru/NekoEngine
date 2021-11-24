#define Handle __EngineHandle

#include <Engine/Engine.h>

#undef Handle

#import "Inspector.h"
#import "AssetManager.h"
#import "SceneHierarchy.h"
#import "EditorController.h"

static NSAlert *_progressAlert;
static EditorController *_controller;

static inline bool _CreateMenu(void);

bool
Ed_CreateGUI(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	_controller = [[EditorController alloc] init];

	if (!_CreateMenu())
		return false;

	NSRect bounds = [[NSScreen mainScreen] visibleFrame];

	NSWindow *wnd = E_screen;
	[wnd setFrameOrigin: NSMakePoint(bounds.origin.x + 250, bounds.size.height)];

	wnd = [[Inspector alloc] initWithContentRect: NSMakeRect(0, 0, 250, *E_screenHeight)];
	[wnd cascadeTopLeftFromPoint: NSMakePoint(bounds.origin.x + *E_screenWidth + 250, bounds.size.height)];
	[wnd setTitle: @"Inspector"];
	[wnd orderFront: NSApp];
	[_controller setInspectorWindow: wnd];

	wnd = [[SceneHierarchy alloc] initWithContentRect: NSMakeRect(0, 0, 250, *E_screenHeight) inspector: (Inspector *)wnd];
	[wnd cascadeTopLeftFromPoint: NSMakePoint(bounds.origin.x, bounds.size.height)];
	[wnd setTitle: @"Scene Hierarchy"];
	[wnd orderFront: NSApp];
	[_controller setSceneHierarchyWindow: wnd];

	wnd = [[AssetManager alloc] initWithContentRect: NSMakeRect(0, 0, 600, 300)];
	[wnd cascadeTopLeftFromPoint: NSMakePoint(bounds.origin.x, bounds.origin.y + [wnd frame].size.height)];
	[wnd setTitle: @"Asset Manager"];
	[wnd makeKeyAndOrderFront: NSApp];
	[_controller setAssetManagerWindow: wnd];

	[pool drain];

	return true;
}

void
EdGUI_MessageBox(const char *title, const char *message)
{
	dispatch_async(dispatch_get_main_queue(), ^{
		NSAlert *a = [[NSAlert alloc] init];
		[a addButtonWithTitle: @"OK"];
		[a setMessageText: [NSString stringWithUTF8String: title]];
		[a setInformativeText: [NSString stringWithUTF8String: message]];
		[a runModal];
		[a release];
	});
}

void
EdGUI_ShowProgressDialog(const char *text)
{
	dispatch_async(dispatch_get_main_queue(), ^{
		if (!_progressAlert) {
			_progressAlert = [[NSAlert alloc] init];

			[_progressAlert addButtonWithTitle: @"OK"];
			[_progressAlert.buttons.firstObject setHidden: YES];

			NSProgressIndicator *pi = [[NSProgressIndicator alloc] initWithFrame: NSMakeRect(0, 0, 300, 25)];
			pi.indeterminate = YES;

			_progressAlert.accessoryView = pi;
			_progressAlert.messageText = [NSString stringWithUTF8String: text];
		}

		[(NSProgressIndicator *)[_progressAlert accessoryView] startAnimation: nil];
		[_progressAlert beginSheetModalForWindow: (NSWindow *)E_screen completionHandler: nil];
	});
}

void
EdGUI_UpdateProgressDialog(const char *text)
{
	dispatch_async(dispatch_get_main_queue(), ^{
		_progressAlert.messageText = [NSString stringWithUTF8String: text];
	});
}

void
EdGUI_HideProgressDialog(void)
{
	dispatch_async(dispatch_get_main_queue(), ^{
		[(NSProgressIndicator *)[_progressAlert accessoryView] stopAnimation: nil];
		[_progressAlert.window orderOut: nil];
	});
}

void
Ed_TermGUI(void)
{
	[_controller release];
	[_progressAlert release];
}

static inline bool
_CreateMenu(void)
{
#define SEPARATOR(x) [x addItem: [NSMenuItem separatorItem]]

#define MENU(title) \
	menuItem = [[[NSMenuItem alloc] init] autorelease];	\
	menu = [[[NSMenu alloc] init] autorelease];	\
	[menu setTitle: title]; \
	[menuItem setSubmenu: menu]; \
	[menuItem setTitle: title];	\
	[mainMenu addItem: menuItem]

#define ITEM(title, action) \
	menuItem = [[[NSMenuItem alloc] init] autorelease];	\
	[menuItem setTitle: title];	\
	[menuItem setTarget: _controller];	\
	[menuItem setAction: action];	\
	[menu addItem: menuItem]

#define ITEMS(title, key, action) \
	menuItem = [[[NSMenuItem alloc] init] autorelease];	\
	[menuItem setTitle: title];	\
	[menuItem setKeyEquivalent: key];	\
	[menuItem setTarget: _controller];	\
	[menuItem setAction: action];	\
	[menu addItem: menuItem]

	NSMenu *mainMenu = [[[NSMenu alloc] init] autorelease], *menu;
	NSMenuItem *menuItem;

	// Application Menu
	menuItem = [[[NSMenuItem alloc] init] autorelease];

	[mainMenu addItem: menuItem];
	[NSApp setMainMenu: mainMenu];

	menu = [[[NSMenu alloc] init] autorelease];
	[menuItem setSubmenu: menu];
	{
		ITEM(@"About NekoEditor", @selector(showAboutPanel:));
		SEPARATOR(menu);
		ITEM(@"Preferences", @selector(showEditorPreferences:));
		SEPARATOR(menu);
		ITEMS(@"Quit NekoEditor", @"q", @selector(quit:));
	}

	MENU(@"File");
	{
		ITEMS(@"New Project", @"n", @selector(newProject:));
		SEPARATOR(menu);
		ITEMS(@"Open Project", @"o", @selector(openProject:));
		ITEMS(@"Save Project", @"s", @selector(saveProject:));
		SEPARATOR(menu);
		ITEMS(@"Close Project", @"w", @selector(closeProject:));
	}

	MENU(@"Project");
	{
		ITEMS(@"Settings", @"n", @selector(showProjectSettings:));
	}

	MENU(@"Tools");
	{
		ITEMS(@"Scene Hierarchy", @"h", @selector(showSceneHierarchy:));
		ITEMS(@"Asset Manager", @"m", @selector(showAssetManager:));
		ITEMS(@"Inspector", @"i", @selector(showInspector:));
	}

	MENU(@"Help");
	{
		ITEM(@"Native API Reference", @selector(showNativeReference:));
		ITEM(@"Scripting API Reference", @selector(showScriptingReference:));
	}

	return true;
}
