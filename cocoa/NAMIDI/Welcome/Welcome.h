//
//  Welcome.h
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Welcome : NSObject
+ (Welcome *)sharedInstance;
- (void)showWindow;
- (void)closeWindow;
@end
