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

@property (assign, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@end

@interface MeasureViewController ()
@property (strong, nonatomic) IBOutlet MeasureView *measureView;
@end

@implementation MeasureViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    _measureView.scaleAssistant = _scaleAssistant;
    _measureView.sequence = [[SequenceRepresentation alloc] init];
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    if (![_scaleAssistant scrollWheel:theEvent]) {
        [super scrollWheel:theEvent];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context
{
    if (object == _scaleAssistant) {
        [_measureView layout];
    }
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

- (void)setSequence:(SequenceRepresentation *)sequence
{
    _sequence = sequence;
    [self layout];
}

- (void)layout
{
    self.frame = CGRectMake(0, 0, _scaleAssistant.pixelPerTick * _sequence.length + _scaleAssistant.measureOffset * 2, self.bounds.size.height);
    [super layout];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    
    [self drawConductorLine:dirtyRect context:ctx];
    [self drawMeasure:dirtyRect context:ctx];
    [self drawEvent:dirtyRect context:ctx];
}

- (void)drawConductorLine:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 1.0);
    CGContextSetStrokeColorWithColor(ctx, _grayColor);
    
    CGContextMoveToPoint(ctx, 0, CONDUCTOR_LINE_Y);
    CGContextAddLineToPoint(ctx, self.bounds.size.width, CONDUCTOR_LINE_Y);
    CGContextStrokePath(ctx);
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

- (void)drawMeasure:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 1.0);
    CGContextSetStrokeColorWithColor(ctx, _grayColor);
    
    CGFloat tickPerPixel = _scaleAssistant.tickPerPixel;
    CGFloat pixelPerTick = _scaleAssistant.pixelPerTick;
    CGFloat measureOffset = _scaleAssistant.measureOffset;
    
    int tick = (dirtyRect.origin.x) * tickPerPixel;
    int tickTo = (dirtyRect.origin.x + dirtyRect.size.width) * tickPerPixel;
    
    Location location = [_sequence locationByTick:tick];
    location.t = 0;
    tick = [_sequence tickByLocation:location];
    TimeSign timeSign = [_sequence timeSignByTick:tick];
    
    while (tick <= tickTo && tick <= _sequence.length) {
        bool isMeasure = 1 == location.b;
        
        CGFloat x = round(tick * pixelPerTick) + measureOffset;
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
    
    CGFloat pixelPerTick = _scaleAssistant.pixelPerTick;
    CGFloat measureOffset = _scaleAssistant.measureOffset;
    int lastTick = -1;
    
    for (MidiEventRepresentation *event in _sequence.eventsOfConductorTrack) {
        int tick = event.tick;
        if (lastTick != tick) {
            CGFloat x = round(tick * pixelPerTick) + measureOffset;
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