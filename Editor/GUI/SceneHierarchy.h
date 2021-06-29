#ifndef _NE_EDITOR_GUI_SCENE_HIERARCHY_H_
#define _NE_EDITOR_GUI_SCENE_HIERARCHY_H_

#import <Cocoa/Cocoa.h>

@interface SceneHierarchy : NSWindow
{
	NSOutlineView *_sceneView;
}

- (id)initWithContentRect:(NSRect)contentRect;

@end

#endif /* _NE_EDITOR_GUI_SCENE_HIERARCHY_H_ */
