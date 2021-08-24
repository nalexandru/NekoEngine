#define Handle __EngineHandle

#include <Scene/Scene.h>
#include <Engine/Entity.h>
#include <Runtime/Runtime.h>

#undef Handle

#import "Inspector.h"

@implementation Inspector

- (id)initWithContentRect: (NSRect)contentRect;
{
	if (!(self = [super initWithContentRect: contentRect
								  styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
									backing: NSBackingStoreBuffered
									  defer: YES]))
		return nil;

	_titleText = [[[NSTextField alloc] initWithFrame: NSMakeRect(10, 10, 230, 25)] autorelease];
	[_titleText setBordered: NO];
	[_titleText setEditable: NO];
	[_titleText setSelectable: NO];

	[[self contentView] addSubview: _titleText];

	return self;
}

- (void)inspectScene
{
	[_titleText setStringValue: [NSString stringWithUTF8String: Rt_WcsToMbs(Scn_activeScene->name)]];
}

- (void)inspectEntity: (void *)entity
{
	if (entity) {
		[_titleText setStringValue: [NSString stringWithUTF8String: Rt_WcsToMbs(E_EntityName(entity))]];
	} else {
		[_titleText setStringValue: @"No selection"];
	}
}

- (void)dealloc
{
	[super dealloc];
}

@end
