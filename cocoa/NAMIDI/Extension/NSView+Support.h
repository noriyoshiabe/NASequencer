//
//  NSView+Support.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define CGRectPrint(rect) NSLog(@"%s: {{%0.0f, %0.0f}, {%0.0f, %0.0f}", #rect, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height)
# define CGRectFromRectWithXY(rect, x, y) CGRectMake(x, y, rect.size.width, rect.size.height)

@interface NSView (Support)
- (void)showHierarchy;
@end
