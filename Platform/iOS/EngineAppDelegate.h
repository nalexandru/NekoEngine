//
//  AppDelegate.h
//  NekoEngine iOS
//
//  Created by Alexandru Naiman on 13.02.2021.
//

#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>

@interface EngineAppDelegate : UIResponder <UIApplicationDelegate, MTKViewDelegate>

@property (strong, nonatomic) UIWindow *window;

@end

