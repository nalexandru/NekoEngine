#define Handle __EngineHandle

#include <Engine/Entity.h>
#include <Engine/Events.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Runtime/Runtime.h>

#undef Handle

#include <uthash.h>

#import "SceneHierarchy.h"

struct EntityNode
{
	TreeNode *node;
	EntityHandle handle;
	UT_hash_handle hh;
};

static void _SceneActivated(SceneHierarchy *sh, struct Scene *scn);
static void _EntityCreated(SceneHierarchy *sh, EntityHandle eh);
static void _EntityDestroyed(SceneHierarchy *sh, EntityHandle eh);
static void _ComponentCreated(SceneHierarchy *sh, const struct ComponentCreationData *ccd);
static void _ComponentDestroyed(SceneHierarchy *sh, CompHandle ch);
static void _AddTransform(const struct Transform *xform, SceneHierarchy *sh, TreeNode *parentNode);

@implementation SceneHierarchy

@synthesize treeController;
@synthesize entities;
@synthesize rootNode;
@synthesize inspector;

- (id)initWithContentRect: (NSRect)contentRect inspector: (Inspector *)inspector
{
	if (!(self = [super initWithContentRect: contentRect
								  styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
									backing: NSBackingStoreBuffered
									  defer: YES]))
		return nil;

	NSScrollView *scrollView = [[[NSScrollView alloc] initWithFrame: NSMakeRect(5, 5, contentRect.size.width - 10, contentRect.size.height - 10)] autorelease];
	[scrollView setHasHorizontalScroller: YES];
	[scrollView setHasVerticalScroller: YES];
	[scrollView setBorderType: NSBezelBorder];

	_hierarchyView = [[NSOutlineView alloc] initWithFrame: NSMakeRect(0, 0, contentRect.size.width - 10, contentRect.size.height)];
	[_hierarchyView setDelegate: self];
	[scrollView setDocumentView: _hierarchyView];

	[[self contentView] addSubview: scrollView];

	treeController = [[NSTreeController alloc] init];
	entities = [[NSMutableArray alloc] init];

#ifdef __APPLE__
	[treeController setLeafKeyPath: @"leaf"];
#else
	[treeController setLeafPathKey: @"leaf"];
#endif

	[treeController setChildrenKeyPath: @"children"];

	NSTableColumn *tc = [[NSTableColumn alloc] initWithIdentifier: @"text"];
#ifdef __APPLE__
	[tc setTitle: @"Entity"];
#endif
	[tc bind: NSValueBinding toObject: treeController withKeyPath: @"arrangedObjects.text" options: nil];
	[_hierarchyView addTableColumn: tc];
	[_hierarchyView sizeLastColumnToFit];

	[treeController bind: NSContentArrayBinding toObject: self withKeyPath: @"entities" options: @{NSRaisesForNotApplicableKeysBindingOption: @YES, NSConditionallySetsEditableBindingOption: @YES}];

#ifdef __APPLE__
	[_hierarchyView bind: NSContentBinding toObject: treeController withKeyPath: @"arrangedObjects" options: @{NSAlwaysPresentsApplicationModalAlertsBindingOption: @YES}];
	[_hierarchyView bind: NSSelectionIndexPathsBinding toObject: treeController withKeyPath: @"selectionIndexPaths" options: @{}];
#else
	[_hierarchyView bind: NSContentBinding toObject: treeController withKeyPath: @"arrangedObjects" options: @{}];
	[_hierarchyView bind: NSSelectionIndexesBinding toObject: treeController withKeyPath: @"selectionIndexPaths" options: @{}];
#endif

	[_hierarchyView bind: NSSortDescriptorsBinding toObject: treeController withKeyPath: @"sortDescriptors" options: @{}];

	_entityLock = [[NSLock alloc] init];

	self.inspector = inspector;

	_entityNodes = NULL;
	_sceneActivatedHandler = E_RegisterHandler(EVT_SCENE_ACTIVATED, (EventHandlerProc)_SceneActivated, self);

	_entityCreatedHandler = E_RegisterHandler(EVT_ENTITY_CREATED, (EventHandlerProc)_EntityCreated, self);
	_entityDestroyedHandler = E_RegisterHandler(EVT_ENTITY_DESTROYED, (EventHandlerProc)_EntityDestroyed, self);

	_componentCreatedHandler = E_RegisterHandler(EVT_COMPONENT_CREATED, (EventHandlerProc)_ComponentCreated, self);
	_componentDestroyedHandler = E_RegisterHandler(EVT_COMPONENT_DESTROYED, (EventHandlerProc)_ComponentDestroyed, self);

	return self;
}

- (void)reloadData
{
	[treeController rearrangeObjects];
	[_hierarchyView reloadData];
}

- (void)lock
{
	[_entityLock lock];
}

- (void)unlock
{
	[_entityLock unlock];
}

- (void)addNode: (TreeNode *)node forHandle: (void *)handle
{
	struct EntityNode *en = malloc(sizeof(*en));
	en->node = node;
	en->handle = handle;
	HASH_ADD_PTR(_entityNodes, node, en);
}

- (struct EntityNode *)findNode: (void *)handle
{
	struct EntityNode *node = _entityNodes;
	while (node) {
		if (node->handle == handle)
			break;

		node = node->hh.next;
	}

	return node;
}

- (void)removeNodeForHandle: (void *)handle
{
	[self removeNode: [self findNode: handle]];
}

- (void)removeNode: (struct EntityNode *)node
{
	HASH_DEL(_entityNodes, node);
}

- (void)outlineViewSelectionDidChange: (NSNotification *)notification
{
	if (![[treeController selectedObjects] count]) {
		[inspector inspectEntity: nil];
		return;
	}

	TreeNode *tn = [[treeController selectedObjects] objectAtIndex: 0];
	if (tn.handle)
		[inspector inspectEntity: tn.handle];
	else
		[inspector inspectScene];
}

- (void)dealloc
{
	E_UnregisterHandler(_sceneActivatedHandler);

	[_hierarchyView release];
	[treeController release];
	[entities release];

	[super dealloc];
}

@end

@implementation TreeNode

@synthesize children;
@synthesize text;
@synthesize handle;

- (id)init
{
	self = [super init];

	children = [[NSMutableArray alloc] init];

	return self;
}

- (BOOL)leaf
{
	return !children.count;
}

- (void)dealloc
{
	[children release];
	[text release];

	[super dealloc];
}

@end

static void
_SceneActivated(SceneHierarchy *sh, struct Scene *scn)
{
	const struct Array *transforms = E_GetAllComponentsS(scn, E_ComponentTypeId(TRANSFORM_COMP));

	if (sh.rootNode) {
		//
	}

	sh.rootNode = [[TreeNode alloc] init];
	sh.rootNode.text = [NSString stringWithFormat:@"%s", Rt_WcsToMbs(scn->name)];
	sh.rootNode.handle = NULL;

	const struct Transform *xform = NULL;
	Rt_ArrayForEach(xform, transforms)
		if (!xform->parent)
			_AddTransform(xform, sh, sh.rootNode);

	[sh.treeController insertObject: sh.rootNode atArrangedObjectIndexPath: [NSIndexPath indexPathWithIndex: 0]];

	dispatch_async(dispatch_get_main_queue(), ^{
		[sh reloadData];
	});
}

static void
_EntityCreated(SceneHierarchy *sh, EntityHandle eh)
{
	const struct Transform *xform = E_GetComponent(eh, E_ComponentTypeId(TRANSFORM_COMP));
	if (!xform)
		return;

	TreeNode *parent = NULL;
	if (!xform->parent)
		parent = sh.rootNode;
	else
		parent = [sh findNode: xform->parent->_owner]->node;

	_AddTransform(xform, sh, parent);

	dispatch_async(dispatch_get_main_queue(), ^{
		[sh reloadData];
	});
}

static void
_EntityDestroyed(SceneHierarchy *sh, EntityHandle eh)
{
	[sh lock];

	/*struct TreeNode *node = _FindNode(eh);
	if (!node)
		return;

	XtUnmanageChild(node->widget);
	XtDestroyWidget(node->widget);
	 */

	[sh removeNodeForHandle: eh];

	dispatch_async(dispatch_get_main_queue(), ^{
		[sh reloadData];
	});

	[sh unlock];
}

static void
_ComponentCreated(SceneHierarchy *sh, const struct ComponentCreationData *ccd)
{
	if (ccd->type != E_ComponentTypeId(TRANSFORM_COMP))
		return;

	_EntityCreated(sh, ccd->owner);
}

static void
_ComponentDestroyed(SceneHierarchy *sh, CompHandle ch)
{
	//if (ccd->type != E_ComponentTypeId(TRANSFORM_COMP))
	//	return;

	// TODO
	//_EntityCreate(NULL, ccd->owner);
}

static void
_AddTransform(const struct Transform *xform, SceneHierarchy *sh, TreeNode *parentNode)
{
	[sh lock];

	if ([sh findNode: xform->_owner]) {
		[sh unlock];
		return;
	}

	TreeNode *node = [[TreeNode alloc] init];
	node.text = [NSString stringWithFormat:@"%s", Rt_WcsToMbs(E_EntityName(xform->_owner))];
	node.handle = xform->_owner;

	[sh addNode: node forHandle: xform->_owner];

	const struct Transform *child = NULL;
	Rt_ArrayForEach(child, &xform->children)
		_AddTransform(child, sh, node);

	[[parentNode children] addObject: node];

	[sh unlock];
}
