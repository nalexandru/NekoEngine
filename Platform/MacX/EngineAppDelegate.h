//
//  EngineAppDelegate.h
//  NekoEngine
//
//  Created by Alexandru Naiman on 9/28/20.
//  Copyright 2020 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
@interface EngineAppDelegate : NSObject <NSApplicationDelegate>
#else
@interface EngineAppDelegate : NSObject
#endif

@end
