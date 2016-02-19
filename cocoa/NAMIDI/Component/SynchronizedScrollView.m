//
//  SynchronizedScrollView.m
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynchronizedScrollView.h"

@interface SynchronizedScrollViewObserveInfo : NSObject 
@property (assign, nonatomic) BOOL x;
@property (assign, nonatomic) BOOL y;
- (instancetype)initWithX:(BOOL)x Y:(BOOL)y;
@end

@implementation SynchronizedScrollViewObserveInfo

- (instancetype)initWithX:(BOOL)x Y:(BOOL)y
{
    self = [super init];
    if (self) {
        _x = x;
        _y = y;
    }
    return self;
}

@end

@interface SynchronizedScrollView () {
    NSMapTable *_observeMap;
}

@end

@implementation SynchronizedScrollView

- (void)awakeFromNib
{
    _userInteractionEnabled = YES;
    _observeMap = [NSMapTable weakToStrongObjectsMapTable];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (NSView *)hitTest:(NSPoint)aPoint
{
    return _userInteractionEnabled ? [super hitTest:aPoint] : nil;
}

- (void)observeScrollForScrollView:(NSScrollView *)scrollView x:(BOOL)x y:(BOOL)y
{
    [_observeMap setObject:[[SynchronizedScrollViewObserveInfo alloc] initWithX:x Y:y] forKey:scrollView.contentView];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewContentBoundsDidChange:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:scrollView.contentView];
}

- (void)scrollViewContentBoundsDidChange:(NSNotification *)notification
{
    NSClipView *changedContentView = notification.object;
    if (changedContentView) {
        SynchronizedScrollViewObserveInfo *info = [_observeMap objectForKey:changedContentView];
        NSPoint changedBoundsOrigin = changedContentView.bounds.origin;
        NSPoint offset = self.contentView.bounds.origin;
        if (info.x && info.y) {
            if (changedBoundsOrigin.x != offset.x || changedBoundsOrigin.y != offset.y) {
                [self.contentView scrollToPoint:changedBoundsOrigin];
                [self reflectScrolledClipView:self.contentView];
            }
        }
        else if (info.x) {
            if (changedBoundsOrigin.x != offset.x) {
                changedBoundsOrigin.y = offset.y;
                [self.contentView scrollToPoint:changedBoundsOrigin];
                [self reflectScrolledClipView:self.contentView];
            }
        }
        else if (info.y) {
            if (changedBoundsOrigin.y != offset.y) {
                changedBoundsOrigin.x = offset.x;
                [self.contentView scrollToPoint:changedBoundsOrigin];
                [self reflectScrolledClipView:self.contentView];
            }
        }
    }
}

@end
