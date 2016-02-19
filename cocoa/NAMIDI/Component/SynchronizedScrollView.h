//
//  SynchronizedScrollView.h
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SynchronizedScrollView : NSScrollView
@property (assign, nonatomic) BOOL userInteractionEnabled;
- (void)observeScrollForScrollView:(NSScrollView *)scrollView x:(BOOL)x y:(BOOL)y;
@end
