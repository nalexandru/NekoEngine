#ifndef _NE_EDITOR_GUI_SCENE_HIERARCHY_H_
#define _NE_EDITOR_GUI_SCENE_HIERARCHY_H_

#import <Cocoa/Cocoa.h>

#import "Inspector.h"

struct EntityNode;

@interface TreeNode : NSObject {}

@property (retain) NSMutableArray *children;
@property (retain) NSString *text;
@property void *handle;

- (BOOL)leaf;

@end

@interface SceneHierarchy : NSWindow<NSOutlineViewDelegate>
{
	NSOutlineView *_hierarchyView;
	uint64_t _sceneActivatedHandler, _entityCreatedHandler, _entityDestroyedHandler,
		_componentCreatedHandler, _componentDestroyedHandler;
	struct EntityNode *_entityNodes;
	NSLock *_entityLock;
}

@property (nonatomic, retain) NSTreeController *treeController;
@property (nonatomic, retain) NSMutableArray *entities;
@property (atomic, retain) TreeNode *rootNode;
@property (nonatomic, weak) Inspector *inspector;

- (id)initWithContentRect: (NSRect)contentRect inspector: (Inspector *)inspector;
- (void)reloadData;

- (void)lock;
- (void)unlock;

- (void)addNode: (TreeNode *)node forHandle: (void *)handle;
- (struct EntityNode *)findNode: (void *)handle;
- (void)removeNodeForHandle: (void *)handle;
- (void)removeNode: (struct EntityNode *)node;

@end

#endif /* _NE_EDITOR_GUI_SCENE_HIERARCHY_H_ */
