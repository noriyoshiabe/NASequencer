//
//  SynchronizedScrollView.h
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SynchronizedScrollView;
@protocol SynchronizedScrollViewDelegate <NSObject>
- (BOOL)synchronizedScrollView:(SynchronizedScrollView *)scrollView shouldMouseDown:(NSEvent *)theEvent;
@end

@interface SynchronizedScrollView : NSScrollView
@property (weak, nonatomic) id<SynchronizedScrollViewDelegate> delegate;
@property (assign, nonatomic) BOOL userInteractionEnabled;
- (void)observeScrollForScrollView:(NSScrollView *)scrollView x:(BOOL)x y:(BOOL)y;
@end
