//
//  PlayLineViewController.m
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PlayLineViewController.h"
#import "Stub.h"

@import QuartzCore;

@interface PlayLineView : NSView
@property (weak, nonatomic) NSView *containerView;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
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
    _playLineView.sequence = [[SequenceRepresentation alloc] init];
    _playLineView.player = [[PlayerRepresentation alloc] init];
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
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
    self.layer.backgroundColor = [NSColor yellowColor].CGColor;
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
    [self update];
}

- (void)layout
{
    self.frame = CGRectMake(0, 0, _scaleAssistant.pixelPerTick * _sequence.length + _scaleAssistant.measureOffset * 2, _containerView.frame.size.height);
    
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    self.layer.bounds = CGRectMake(0, 0, 1, _containerView.frame.size.height);
    [CATransaction commit];
    
    [super layout];
    
    [self update];
}

- (void)update
{
    CGFloat x = round(_player.tick * _scaleAssistant.pixelPerTick) + _scaleAssistant.measureOffset - 0.5;
    [CATransaction begin];
    [CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
    self.layer.transform = CATransform3DMakeTranslation(x, 0, 0);
    [CATransaction commit];
}

@end
