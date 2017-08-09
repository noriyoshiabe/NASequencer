//
//  PlayLineViewController.m
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PlayLineViewController.h"

@import QuartzCore;

@interface PlayLineView : NSView <PlayerRepresentationObserver> {
    BOOL _scrolling;
    CALayer *_lineLayer;
}
@property (weak, nonatomic) NSScrollView *containerView;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) MainWindowContext *mainWindowContext;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@property (strong, nonatomic) PlayerRepresentation *player;
@property (assign, nonatomic) int tick;
@end

@interface PlayLineViewController ()
@property (strong) IBOutlet PlayLineView *playLineView;
@end

@implementation PlayLineViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    _playLineView.containerView = _containerView;
    _playLineView.scaleAssistant = _scaleAssistant;
    _playLineView.mainWindowContext = _mainWindowContext;
    _playLineView.sequence = _namidi.sequence;
    _playLineView.player = _namidi.player;
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
}

- (void)dealloc
{
    [_scaleAssistant removeObserver:self forKeyPath:@"scale"];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context
{
    if (object == _scaleAssistant) {
        [_playLineView layout];
    }
}

@end

@implementation PlayLineView

- (void)awakeFromNib
{
    _lineLayer = [CALayer layer];
    _lineLayer.backgroundColor = [NSColor yellowColor].CGColor;
    [self.layer addSublayer:_lineLayer];
}

- (void)dealloc
{
    [_player removeObserver:self];
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)setSequence:(SequenceRepresentation *)sequence
{
    _sequence = sequence;
    [self layout];
}

- (void)setPlayer:(PlayerRepresentation *)player
{
    _player = player;
    [_player addObserver:self];
    [self update];
}

- (void)layout
{
    self.frame = CGRectMake(0, 0, _scaleAssistant.viewWidth, _containerView.frame.size.height);
    
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    _lineLayer.bounds = CGRectMake(0, 0, 1, _containerView.frame.size.height);
    [CATransaction commit];
    
    [super layout];
    
    [self update];
}

- (void)update
{
    CGFloat x = round(_player.tick * _scaleAssistant.pixelPerTick) + _scaleAssistant.measureOffset - 0.5;
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    _lineLayer.transform = CATransform3DMakeTranslation(x, 0, 0);
    [CATransaction commit];
}

- (void)scrollToCurrent
{
    if (_scrolling) {
        return;
    }
    
    CGFloat x = roundf(_player.tick * _scaleAssistant.pixelPerTick + _scaleAssistant.measureOffset);
    
    if (!CGRectIntersectsRect(_containerView.documentVisibleRect, CGRectMake(x, 0, 0, self.bounds.size.height))) {
        _scrolling = YES;
        
        [NSAnimationContext runAnimationGroup:^(NSAnimationContext * _Nonnull context) {
            context.duration = 0.5;
            CGPoint point = CGPointMake(x - _scaleAssistant.measureOffset, _containerView.documentVisibleRect.origin.y);
            [_containerView.contentView.animator setBoundsOrigin:point];
        } completionHandler:^{
            _scrolling = false;
            [self scrollToCurrent];
        }];
    }
}

#pragma mark PlayerRepresentationObserver

- (void)player:(PlayerRepresentation *)player onNotifyClock:(int)tick usec:(int64_t)usec location:(Location)location
{
    [self update];
    
    if (_mainWindowContext.autoScrollEnabled) {
        [self scrollToCurrent];
    }
}

@end
