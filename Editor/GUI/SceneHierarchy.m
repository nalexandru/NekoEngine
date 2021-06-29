#define Handle __EngineHandle

#include <Engine/Entity.h>

#undef Handle

#import "SceneHierarchy.h"

@interface SceneHierarchyDataSource : NSObject<NSOutlineViewDataSource>
@end

@interface SceneHierarchyDelegate : NSObject<NSOutlineViewDelegate>
@end

@implementation SceneHierarchy

- (id)initWithContentRect: (NSRect)contentRect;
{
	if (!(self = [super initWithContentRect: contentRect
								  styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
									backing: NSBackingStoreBuffered
									  defer: YES]))
		return nil;

	_sceneView = [[NSOutlineView alloc] initWithFrame: NSMakeRect(10, 10, contentRect.size.width - 20, contentRect.size.height - 20)];
	[_sceneView setDataSource: [[SceneHierarchyDataSource alloc] init]];

	return self;
}

- (void)dealloc
{
	[_sceneView release];

	[super dealloc];
}

@end

@implementation SceneHierarchyDataSource

- (id)init
{
	self = [super init];

//	NSDictionary *firstParent = @{@"parent": @"Foo", @"children": @[@"Foox", @"Fooz"]};
//	NSDictionary *secondParent = @{@"parent": @"Bar", @"children": @[@"Barx", @"Barz"]};
//	_entities = @[firstParent, secondParent];

	return self;
}

- (id)outlineView: (NSOutlineView *)outlineView child: (NSInteger)index ofItem: (id)item
{
/*	if (!item)
		return [_entities objectAtIndex: index];

	if ([item isKindOfClass: [NSDictionary class]])
		return [[item objectForKey: @"children"] objectAtIndex: index];*/

	return nil;
}

- (BOOL)outlineView: (NSOutlineView *)outlineView isItemExpandable: (id)item
{
	return YES;
}

- (NSInteger)outlineView: (NSOutlineView *)outlineView numberOfChildrenOfItem: (id)item
{
	//if (!item)
	//	E_EntityCount();

	
	/*if (!item)
		return [_entities count];

	if ([item isKindOfClass: [NSDictionary class]])
		return [[item objectForKey: @"children"] count];*/

	return 0;
}

- (id)outlineView: (NSOutlineView *)outlineView objectValueForTableColumn: (NSTableColumn *)tableColumn byItem: (id)item
{
	return @"Barzoi";
}

- (void)outlineView: (NSOutlineView *)outlineView setObjectValue: (id)object forTableColumn: (NSTableColumn *)tableColumn byItem: (id)item
{

}

@end

@implementation SceneHierarchyDelegate
@end
