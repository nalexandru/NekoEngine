#ifndef _NE_EDITOR_GUI_COCOA_ASSET_MANAGER_H_
#define _NE_EDITOR_GUI_COCOA_ASSET_MANAGER_H_

#import <Cocoa/Cocoa.h>

@interface AssetManagerData : NSObject<NSTableViewDataSource, NSTableViewDelegate>
{
	NSMutableArray *_items;
	NSString *_rootPath, *_currentPath;
}

- (id)initWithPath: (NSString *)path;
- (void)changeDir: (NSString *)relativePath;
- (void)update;
- (BOOL)execute: (NSInteger)index;
- (NSString *)currentPath;
- (void)dealloc;

@end

@interface AssetManager : NSWindow
{
	NSTableView *_assetView;
	AssetManagerData *_data;
}

- (id)initWithContentRect:(NSRect)contentRect;

- (void)tableDoubleClick: (id)sender;

- (void)loadAssetList;
- (void)importAsset: (id)sender;

@end

#endif /* _NE_EDITOR_GUI_COCOA_ASSET_MANAGER_H_ */
