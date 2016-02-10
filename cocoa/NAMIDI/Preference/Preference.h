//
//  Preference.h
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Preference : NSObject
+ (Preference *)sharedInstance;
- (void)showWindow;
@end
