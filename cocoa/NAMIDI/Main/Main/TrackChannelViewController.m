//
//  TrackChannelViewController.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TrackChannelViewController.h"
#import "Color.h"

#define NOTE_OFFSET_Y 10.5

@interface TrackChannelView : NSView {
    CGColorRef _blackColor;
    CGColorRef _whiteColor;
    CGColorRef _outsideEdgeColor;
    CGColorRef _insideEdgeColor;
    CGColorRef _outsideEdgeInverseColor;
    
    CGGradientRef _gradient;
    CGGradientRef _gradientInverse;
}

@property (assign, nonatomic) int channel;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (assign, nonatomic) NSColor *baseColor;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@end

@interface TrackChannelViewController ()  <NAMidiRepresentationObserver>
@property (strong) IBOutlet TrackChannelView *trackChannelView;
@end

@implementation TrackChannelViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    _trackChannelView.baseColor = [Color channelColor:_channel];
    _trackChannelView.channel = _channel;
    _trackChannelView.scaleAssistant = _scaleAssistant;
    _trackChannelView.trackSelection = _trackSelection;
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    _trackChannelView.sequence = _namidi.sequence;
    
    [_trackSelection addObserver:self forKeyPath:@"selectionFlags" options:0 context:NULL];
    [_namidi addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    [_trackSelection removeObserver:self forKeyPath:@"selectionFlags"];
    [_namidi removeObserver:self];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context
{
    if (object == _trackSelection) {
        if ([_trackSelection isTrackSelectionChanged:_channel]) {
            _trackChannelView.needsDisplay = YES;
        }
    }
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    _trackChannelView.sequence = sequence;
}

@end

@implementation TrackChannelView

- (void)awakeFromNib
{
    _blackColor = [NSColor blackColor].CGColor;
    _whiteColor = [NSColor whiteColor].CGColor;
}

- (void)dealloc
{
    if (_gradient) {
        CGGradientRelease(_gradient);
    }
    
    if (_gradientInverse) {
        CGGradientRelease(_gradientInverse);
    }
    
    if (_outsideEdgeColor) {
        CGColorRelease(_outsideEdgeColor);
    }
    
    if (_insideEdgeColor) {
        CGColorRelease(_insideEdgeColor);
    }
    
    if (_outsideEdgeInverseColor) {
        CGColorRelease(_outsideEdgeInverseColor);
    }
}

- (BOOL)isFlipped
{
    return NO;
}

- (void)setBaseColor:(NSColor *)baseColor
{
    _baseColor = baseColor;
    
    _outsideEdgeColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                             saturation:_baseColor.saturationComponent
                                             brightness:_baseColor.brightnessComponent * 0.7
                                                  alpha:_baseColor.alphaComponent].CGColor;
    _insideEdgeColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                            saturation:_baseColor.saturationComponent * 0.3
                                            brightness:_baseColor.brightnessComponent
                                                 alpha:_baseColor.alphaComponent].CGColor;
    _outsideEdgeInverseColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                                    saturation:_baseColor.saturationComponent
                                                    brightness:_baseColor.brightnessComponent * 0.2
                                                         alpha:_baseColor.alphaComponent].CGColor;
    
    CGColorRetain(_outsideEdgeColor);
    CGColorRetain(_insideEdgeColor);
    CGColorRetain(_outsideEdgeInverseColor);
    
    if (!_gradient) {
        NSColor *topColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                                 saturation:_baseColor.saturationComponent * 0.4
                                                 brightness:_baseColor.brightnessComponent
                                                      alpha:_baseColor.alphaComponent];
        NSColor *bottomColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                                    saturation:_baseColor.saturationComponent * 0.45
                                                    brightness:_baseColor.brightnessComponent
                                                         alpha:_baseColor.alphaComponent];
        
        _gradient = [NSColor createVerticalGradientWithTopColor:topColor bottomColor:bottomColor];
        
    }
    
    if (!_gradientInverse) {
        NSColor *topColor = [NSColor colorWith8bitRed:85 green:87 blue:93 alpha:255];
        NSColor *bottomColor = [NSColor colorWith8bitRed:27 green:30 blue:38 alpha:255];
        _gradientInverse = [NSColor createVerticalGradientWithTopColor:topColor bottomColor:bottomColor];
    }
}

- (void)setSequence:(SequenceRepresentation *)sequence
{
    _sequence = sequence;
    
    self.needsLayout = YES;
    self.needsDisplay = YES;
}

- (void)layout
{
    self.frame = CGRectMake(0, self.frame.origin.y, _scaleAssistant.viewWidth, self.bounds.size.height);
    [super layout];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    if (_sequence) {
        CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
        [self drawEdge:dirtyRect context:ctx];
        [self drawBody:dirtyRect context:ctx];
        [self drawNotes:dirtyRect context:ctx];
    }
}

- (void)drawEdge:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 1.0);
    
    CGContextSetStrokeColorWithColor(ctx, [_trackSelection isTrackSelected:_channel] ? _outsideEdgeInverseColor :_outsideEdgeColor);
    CGContextAddRect(ctx, CGRectInset(self.bounds, 0.5, 0.5));
    CGContextStrokePath(ctx);
    
    CGContextSetStrokeColorWithColor(ctx, _insideEdgeColor);
    CGContextAddRect(ctx, CGRectInset(self.bounds, 1.5, 1.5));
    CGContextStrokePath(ctx);
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

- (void)drawBody:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextAddRect(ctx, CGRectInset(self.bounds, 2, 2));
    CGContextClip(ctx);
    
    CGContextDrawLinearGradient(ctx,
                                [_trackSelection isTrackSelected:_channel] ? _gradientInverse : _gradient,
                                CGPointMake(0, 0),
                                CGPointMake(0, dirtyRect.size.height),
                                kCGGradientDrawsAfterEndLocation);
    
    CGContextRestoreGState(ctx);
}

- (void)drawNotes:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetStrokeColorWithColor(ctx, [_trackSelection isTrackSelected:_channel] ? _whiteColor : _blackColor);
    
    CGFloat pixelPerTick = _scaleAssistant.pixelPerTick;
    CGFloat measureOffset = _scaleAssistant.measureOffset;
    
    ChannelRepresentation *channel = _sequence.channels[_channel - 1];
    
    CGFloat centerY = CGRectGetMidY(self.bounds);
    CGFloat heightPerKey = (self.bounds.size.height - NOTE_OFFSET_Y * 2) / MAX(12.0, channel.noteRange.high - channel.noteRange.low);
    int centerNoteNo = (channel.noteRange.high + channel.noteRange.low) / 2;
    
    for (MidiEventRepresentation *event in channel.events) {
        if (MidiEventTypeNote == event.type) {
            NoteEvent *note = (NoteEvent *)event.raw;
            CGFloat left = round(note->tick * pixelPerTick) + measureOffset;
            CGFloat right = left + round(note->gatetime * pixelPerTick);
            CGFloat y = floor(centerY + heightPerKey * (note->noteNo - centerNoteNo)) + 0.5;
            if (CGRectContainsPoint(dirtyRect, CGPointMake(left, y)) || CGRectContainsPoint(dirtyRect, CGPointMake(right, y))) {
                CGContextMoveToPoint(ctx, left, y);
                CGContextAddLineToPoint(ctx, right, y);
                CGContextStrokePath(ctx);
            }
        }
    }
    
    CGContextRestoreGState(ctx);
}

@end
