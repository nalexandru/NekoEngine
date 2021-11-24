#define Handle __EngineHandle

#include <Engine/Engine.h>
#include <Engine/IO.h>

#include <Editor/GUI.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

#undef Handle

#import "AssetManager.h"

@interface AssetManagerWindowDelegate : NSObject<NSWindowDelegate>
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
	[btn setBezelStyle: NSBezelStyleRounded];

	[view addSubview: btn];

	NSScrollView *scrollView = [[[NSScrollView alloc] initWithFrame: NSMakeRect(5, 5, contentRect.size.width - 10, contentRect.size.height - 40)] autorelease];
	[scrollView setHasHorizontalScroller: YES];
	[scrollView setHasVerticalScroller: YES];
	[scrollView setBorderType: NSBezelBorder];

	_assetView = [[NSTableView alloc] initWithFrame: NSMakeRect(5, 5, contentRect.size.width - 10, contentRect.size.height - 40)];
	[_assetView setColumnAutoresizingStyle: NSTableViewSequentialColumnAutoresizingStyle];

	NSTableColumn *tc = [[NSTableColumn alloc] initWithIdentifier: @"text"];
	[tc setTitle: @"/"];


	[_assetView addTableColumn: tc];

	[scrollView setDocumentView: _assetView];
	[view addSubview: scrollView];

	_data = [[AssetManagerData alloc] initWithPath: nil];
	[_assetView setDelegate: _data];
	[_assetView setDataSource: _data];
	[_assetView setTarget: self];
	[_assetView setDoubleAction: @selector(tableDoubleClick:)];
	[_assetView sizeLastColumnToFit];

	return self;
}

- (void)loadAssetList
{
}

- (void)importAsset: (id)sender
{
	if (Asset_ImportInProgress()) {
		EdGUI_MessageBox("Import in Progress", "Please wait until the current import operation finishes");
		return;
	}

	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	[openPanel setCanChooseFiles: YES];
	[openPanel setCanChooseDirectories: NO];

	if ([openPanel runModal] != NSModalResponseOK)
		return;

	Asset_Import([[[openPanel URL] path] UTF8String]);
}

- (void)tableDoubleClick: (id)sender
{
	NSTableView *table = (NSTableView *)sender;

	if ([_data execute: [table selectedRow]]) {
		[_assetView reloadData];
		[[[_assetView tableColumns] objectAtIndex: 0] setTitle: [_data currentPath]];
	}
}

- (void)dealloc
{
	[super dealloc];
}

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

@implementation AssetManagerData

- (id)initWithPath: (NSString *)path
{
	if (!(self = [super init]))
		return nil;

	if (!path)
		path = @"/";

	_rootPath = _currentPath = path;
	_items = [[NSMutableArray alloc] init];

	[self update];

	return self;
}

- (void)changeDir: (NSString *)newDir
{
	_currentPath = [newDir retain];
	[self update];
}

- (void)update
{
	const char *dir = [_currentPath UTF8String];
	const char **files = E_ListFiles(dir);

	[_items removeAllObjects];

	if (strlen(dir) > 1)
		[_items addObject: @".."];

	for (const char **i = files; *i != NULL; ++i)
		if (*i[0] != '.')
			[_items addObject: [NSString stringWithUTF8String: *i]];

	E_FreeFileList(files);
}

- (BOOL)execute: (NSInteger)index
{
	NSString *item = [_items objectAtIndex: index];
	NSString *path = [item isEqualToString: @".."] ?
		[_currentPath stringByDeletingLastPathComponent]
	:
		[_currentPath stringByAppendingPathComponent: item];

	if (E_IsDirectory([path UTF8String])) {
		[self changeDir: path];
		return YES;
	} else {
		Ed_OpenAsset([path UTF8String]);
		return NO;
	}
}

- (NSString *)currentPath
{
	return _currentPath;
}

- (NSInteger)numberOfRowsInTableView: (NSTableView *)table
{
	return [_items count];
}

- (NSView *)tableView: (NSTableView *)table viewForColumn: (NSTableColumn *)column row: (NSInteger)row
{
	NSTableCellView *cell = [table makeViewWithIdentifier: @"Asset" owner: self];
	if (cell)
		return cell;

	NSTextField *tf = [[NSTextField alloc] init];
	[tf setBackgroundColor: [NSColor clearColor]];
	[tf setTranslatesAutoresizingMaskIntoConstraints: NO];
	[tf setBordered: NO];
	[tf setControlSize: NSControlSizeSmall];

	cell = [[NSTableCellView alloc] init];
	[cell setIdentifier: @"Asset"];
	[cell addSubview: tf];
	[cell setTextField: tf];

	[tf bind: NSValueBinding toObject: cell withKeyPath: @"objectValue" options: nil];

	return cell;
}

- (id)tableView: (NSTableView *)table objectValueForTableColumn: (NSTableColumn *)column row: (NSInteger)row
{
	return [_items objectAtIndex: row];
}

- (void)dealloc
{
	[_items release];
	[_rootPath release];
	[_currentPath release];

	[super dealloc];
}

@end
