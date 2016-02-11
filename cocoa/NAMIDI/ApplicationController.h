//
//  ApplicationController.h
//  NAMIDI
//
//  Created by abechan on 2/11/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ApplicationController : NSObject
+ (ApplicationController *)sharedInstance;
- (void)showAboutWindow;
- (void)showPreferenceWindow;
@end
