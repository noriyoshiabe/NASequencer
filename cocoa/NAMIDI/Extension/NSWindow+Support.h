//
//  NSWindow+Support.h
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSWindow (Support)
- (void)setFlippedFrameTopLeftPoint:(NSPoint)aPoint;
- (BOOL)isFullScreen;
@end
