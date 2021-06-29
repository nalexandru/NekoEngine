#ifndef _NE_EDITOR_GUI_EDITOR_CONTROLLER_H_
#define _NE_EDITOR_GUI_EDITOR_CONTROLLER_H_

#import <Cocoa/Cocoa.h>

@interface EditorController : NSObject

@property(retain, nonatomic) NSWindow *assetManagerWindow;
@property(retain, nonatomic) NSWindow *sceneHierarchyWindow;
@property(retain, nonatomic) NSWindow *inspectorWindow;

// Application Menu
- (void)showAboutPanel: (id)sender;
- (void)showEditorPreferences: (id)sender;
- (void)quit: (id)sender;

// File Menu
- (void)newProject: (id)sender;
- (void)openProject: (id)sender;
- (void)saveProject: (id)sender;
- (void)closeProject: (id)sender;

// Project Menu
- (void)showProjectSettings: (id)sender;

// Tools Menu
- (void)showSceneHierarchy: (id)sender;
- (void)showAssetManager: (id)sender;
- (void)showInspector: (id)sender;

// Help Menu
- (void)showNativeReference: (id)sender;
- (void)showScriptingReference: (id)sender;

@end

#endif /* _NE_EDITOR_GUI_EDITOR_CONTROLLER_H_ */
