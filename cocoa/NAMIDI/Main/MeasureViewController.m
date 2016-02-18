//
//  MeasureViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MeasureViewController.h"
#import "Color.h"
#import "Stub.h"

#define MEASURE_OFFSET 10.5
#define MEASURE_LINE_HEIGHT 15.0
#define BEAT_LINE_HEIGHT 10.0
#define CONDUCTOR_LINE_Y 36.5
#define MEASURE_NO_Y 20.0
#define EVENT_Y      46.0
#define EVENT_RADIUS 4.0

@interface MeasureView : NSView {
    CGColorRef _grayColor;
    CGColorRef _whiteColor;
    CGColorRef _lightGrayColor;
    NSDictionary *_measureNumberAttrs;
}

@property (assign, nonatomic) CGFloat scale;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@property (readonly, nonatomic) CGFloat pixelPerTick;
@property (readonly, nonatomic) CGFloat tickPerPixel;
@end

@interface MeasureViewController ()
@property (strong, nonatomic) IBOutlet MeasureView *measureView;
@end

@implementation MeasureViewController
@dynamic scale;

- (void)viewDidLoad
{
    [super viewDidLoad];
    _measureView.scale = 1.0;
    _measureView.sequence = [[SequenceRepresentation alloc] init];
}

- (void)setScale:(CGFloat)scale
{
    _measureView.scale = scale;
}

- (CGFloat)scale
{
    return _measureView.scale;
}

@end

@implementation MeasureView

- (void)awakeFromNib
{
    _grayColor = [Color gray].CGColor;
    _whiteColor = [NSColor whiteColor].CGColor;
    _lightGrayColor = [Color lightGray].CGColor;
    
    _measureNumberAttrs = @{NSFontAttributeName:[NSFont boldSystemFontOfSize:11.0], NSForegroundColorAttributeName: [NSColor whiteColor]};
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
    [self drawMeasure:dirtyRect context:ctx];
    [self drawEvent:dirtyRect context:ctx];
}

- (void)drawMeasure:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 1.0);
    CGContextSetStrokeColorWithColor(ctx, _grayColor);
    
    CGContextMoveToPoint(ctx, dirtyRect.origin.x, CONDUCTOR_LINE_Y);
    CGContextAddLineToPoint(ctx, dirtyRect.size.width, CONDUCTOR_LINE_Y);
    CGContextStrokePath(ctx);
    
    CGFloat tickPerPixel = self.tickPerPixel;
    CGFloat pixelPerTick = self.pixelPerTick;
    
    int tick = (dirtyRect.origin.x) * tickPerPixel;
    int tickTo = (dirtyRect.origin.x + dirtyRect.size.width - MEASURE_OFFSET * 2) * tickPerPixel;
    
    Location location = [_sequence locationByTick:tick];
    location.t = 0;
    tick = [_sequence tickByLocation:location];
    TimeSign timeSign = [_sequence timeSignByTick:tick];
    
    while (tick <= tickTo) {
        bool isMeasure = 1 == location.b;
        
        CGFloat x = round(tick * pixelPerTick) + MEASURE_OFFSET;
        CGFloat y = isMeasure ? MEASURE_LINE_HEIGHT : BEAT_LINE_HEIGHT;
        
        CGContextMoveToPoint(ctx, x, 0);
        CGContextAddLineToPoint(ctx, x, y);
        CGContextStrokePath(ctx);
        
        if (isMeasure) {
            CGContextSaveGState(ctx);
            NSString *string = [NSString stringWithFormat:@"%d", location.m];
            CGFloat width = [string sizeWithAttributes:_measureNumberAttrs].width;
            [string drawAtPoint:CGPointMake(x - width / 2.0, MEASURE_NO_Y) withAttributes:_measureNumberAttrs];
            CGContextRestoreGState(ctx);
        }
        
        if (timeSign.numerator < ++location.b) {
            location.b = 1;
            ++location.m;
            timeSign = [_sequence timeSignByTick:tick];
        }
        
        tick = [_sequence tickByLocation:location];
    }
    
    CGContextRestoreGState(ctx);
}

- (void)drawEvent:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetFillColorWithColor(ctx, _lightGrayColor);
    
    CGFloat pixelPerTick = self.pixelPerTick;
    int lastTick = -1;
    
    for (MidiEventRepresentation *event in _sequence.eventsOfConductorTrack) {
        int tick = event.tick;
        if (lastTick != tick) {
            CGFloat x = round(event.tick * pixelPerTick) + MEASURE_OFFSET;
            CGRect rect = CGRectMake(x - EVENT_RADIUS, EVENT_Y - EVENT_RADIUS, EVENT_RADIUS * 2, EVENT_RADIUS * 2);
            if (CGRectIntersectsRect(dirtyRect, rect)) {
                CGContextFillEllipseInRect(ctx, rect);
            }
            lastTick = tick;
        }
    }
    
    CGContextRestoreGState(ctx);
}

@end
