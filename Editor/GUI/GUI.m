#import "AssetManager.h"
#import "SceneHierarchy.h"
#import "EditorController.h"

static EditorController *_controller;

static inline bool _CreateMenu(void);

#ifndef __APPLE__
#import <Cocoa/Cocoa.h>
static inline bool _InitCocoa(void);
#endif

bool
Ed_CreateGUI(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

#ifndef __APPLE__
	if (!_InitCocoa())
		return false;
#endif

	_controller = [[EditorController alloc] init];

	if (!_CreateMenu())
		return false;

	NSWindow *wnd = [[SceneHierarchy alloc] initWithContentRect: NSMakeRect(10, 10, 250, 400)];
	[wnd cascadeTopLeftFromPoint: NSMakePoint(20, 20)];
	[wnd setTitle: @"Scene Hierarchy"];
	[wnd orderFront: NSApp];
	[_controller setSceneHierarchyWindow: wnd];

	wnd = [[AssetManager alloc] initWithContentRect: NSMakeRect(100, 100, 600, 300)];
	[wnd cascadeTopLeftFromPoint: NSMakePoint(20, 20)];
	[wnd setTitle: @"Asset Manager"];
	[wnd makeKeyAndOrderFront: NSApp];
	[_controller setAssetManagerWindow: wnd];

	[pool drain];

	return true;
}

static inline bool
_CreateMenu(void)
{

#ifdef __APPLE__
#	define SEPARATOR(x) [x addItem: [NSMenuItem separatorItem]]
#else
#	define SEPARATOR(x) (void)x;
#endif

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
#ifndef __APPLE__
	[menuItem setTitle: @"Info"];
#endif
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

#ifndef __APPLE__
void
Ed_ProcessCocoaEvents(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	while (1) {
		NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
										untilDate: [NSDate distantPast]
										   inMode: NSDefaultRunLoopMode
										  dequeue: YES];

		if (!e)
			break;

		[NSApp sendEvent: e];
	}

	[pool drain];
}

static inline bool
_InitCocoa(void)
{
	//NSApplicationLoad();
	[NSApplication sharedApplication];
	[NSApp finishLaunching];

	//[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	return true;
}
#endif
