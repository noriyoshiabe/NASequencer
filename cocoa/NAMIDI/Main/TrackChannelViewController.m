//
//  TrackChannelViewController.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TrackChannelViewController.h"
#import "Color.h"
#import "Stub.h"

#define MEASURE_OFFSET 10.5
#define NOTE_OFFSET_Y 10.5

@interface TrackChannelView : NSView {
    CGColorRef _blackColor;
    CGColorRef _outsideEdgeColor;
    CGColorRef _insideEdgeColor;
    CGGradientRef _gradient;
}

@property (assign, nonatomic) int channel;
@property (assign, nonatomic) CGFloat scale;
@property (assign, nonatomic) NSColor *baseColor;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@property (readonly, nonatomic) CGFloat pixelPerTick;
@property (readonly, nonatomic) CGFloat tickPerPixel;
@end

@interface TrackChannelViewController ()
@property (strong) IBOutlet TrackChannelView *trackChannelView;
@end

@implementation TrackChannelViewController
@dynamic scale;

- (void)viewDidLoad
{
    [super viewDidLoad];
    _trackChannelView.baseColor = [Color channelColor:_channel];
    _trackChannelView.channel = _channel;
    _trackChannelView.scale = 1.0;
    _trackChannelView.sequence = [[SequenceRepresentation alloc] init];
}

- (void)setScale:(CGFloat)scale
{
    _trackChannelView.scale = scale;
}

- (CGFloat)scale
{
    return _trackChannelView.scale;
}

@end

@implementation TrackChannelView

- (void)awakeFromNib
{
    _blackColor = [NSColor blackColor].CGColor;
}

- (void)dealloc
{
    if (_gradient) {
        CGGradientRelease(_gradient);
    }
}

- (BOOL)isFlipped
{
    return YES;
}

- (CGFloat)pixelPerTick
{
    return 20.0 / 480.0 * _scale;
}

- (CGFloat)tickPerPixel
{
    return 1.0 / self.pixelPerTick;
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
    
    if (!_gradient) {
        NSColor *topColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                                 saturation:_baseColor.saturationComponent * 0.4
                                                 brightness:_baseColor.brightnessComponent
                                                      alpha:_baseColor.alphaComponent];
        NSColor *bottomColor = [NSColor colorWithCalibratedHue:_baseColor.hueComponent
                                                    saturation:_baseColor.saturationComponent * 0.45
                                                    brightness:_baseColor.brightnessComponent
                                                         alpha:_baseColor.alphaComponent];
        
        CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
        CGFloat components[] = {
            topColor.redComponent, topColor.greenComponent, topColor.blueComponent, topColor.alphaComponent,
            bottomColor.redComponent, bottomColor.greenComponent, bottomColor.blueComponent, bottomColor.alphaComponent,
        };
        CGFloat locations[] = {0.0f, 1.0f};
        size_t count = sizeof(components) / (sizeof(CGFloat) * 4);
        
        _gradient = CGGradientCreateWithColorComponents(colorSpaceRef, components, locations, count);
        
        CGColorSpaceRelease(colorSpaceRef);
    }
}

- (void)setScale:(CGFloat)scale
{
    _scale = scale;
    [self layout];
}

- (void)setSequence:(SequenceRepresentation *)sequence
{
    _sequence = sequence;
    [self layout];
}

- (void)layout
{
    self.frame = CGRectMake(0, 0, self.pixelPerTick * _sequence.length + MEASURE_OFFSET * 2, self.bounds.size.height);
    [super layout];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    [self drawEdge:dirtyRect context:ctx];
    [self drawBody:dirtyRect context:ctx];
    [self drawNotes:dirtyRect context:ctx];
}

- (void)drawEdge:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 1.0);
    
    CGContextSetStrokeColorWithColor(ctx, _outsideEdgeColor);
    CGContextAddRect(ctx, CGRectInset(dirtyRect, 0.5, 0.5));
    CGContextStrokePath(ctx);
    
    CGContextSetStrokeColorWithColor(ctx, _insideEdgeColor);
    CGContextAddRect(ctx, CGRectInset(dirtyRect, 1.5, 1.5));
    CGContextStrokePath(ctx);
    CGContextFillPath(ctx);
    
    CGContextRestoreGState(ctx);
}

- (void)drawBody:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextAddRect(ctx, CGRectInset(dirtyRect, 2, 2));
    CGContextClip(ctx);
    
    CGContextDrawLinearGradient(ctx,
                                _gradient,
                                CGPointMake(0, 0),
                                CGPointMake(0, dirtyRect.size.height),
                                kCGGradientDrawsAfterEndLocation);
    
    CGContextRestoreGState(ctx);
}

- (void)drawNotes:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetStrokeColorWithColor(ctx, _blackColor);
    
    CGFloat pixelPerTick = self.pixelPerTick;
    ChannelRepresentation *channel = _sequence.channels[_channel - 1];
    CGFloat heightPerKey = (self.bounds.size.height - NOTE_OFFSET_Y * 2) / (channel.noteRange.high - channel.noteRange.low);
    
    for (MidiEventRepresentation *event in channel.events) {
        if (MidiEventTypeNote == event.type) {
            NoteEvent *note = (NoteEvent *)event.raw;
            CGFloat left = round(note->tick * pixelPerTick) + MEASURE_OFFSET;
            CGFloat right = left + round(note->gatetime * pixelPerTick);
            CGFloat y = floor(self.bounds.size.height - NOTE_OFFSET_Y - heightPerKey * (note->noteNo - channel.noteRange.low)) + 0.5;
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
