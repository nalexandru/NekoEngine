#ifndef _NE_EDITOR_GUI_ASSET_MANAGER_H_
#define _NE_EDITOR_GUI_ASSET_MANAGER_H_

#import <Cocoa/Cocoa.h>

@interface AssetManager : NSWindow
{
	NSGridView *_assetView;
}

- (id)initWithContentRect:(NSRect)contentRect;

- (void)importAsset: (id)sender;

@end

#endif /* _NE_EDITOR_GUI_ASSET_MANAGER_H_ */
