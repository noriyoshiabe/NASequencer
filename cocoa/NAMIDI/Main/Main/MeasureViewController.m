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
#define MEASURE_NO_Y 20.0
#define BOTTOM_LINE_Y 36.5

@interface MeasureView : NSView {
    CGColorRef _grayColor;
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
    _measureView.sequence = _namidi.sequence;
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
}

- (void)dealloc
{
    [_scaleAssistant removeObserver:self forKeyPath:@"scale"];
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

- (void)mouseDown:(NSEvent *)theEvent
{
    CGPoint point = [self.view convertPoint:theEvent.locationInWindow fromView:self.view.window.contentView];
    
    int tick = (point.x - _scaleAssistant.measureOffset) * _scaleAssistant.tickPerPixel;
    Location location = [_namidi.sequence locationByTick:tick];
    location.t = 0;
    location.b = 1;
    tick = [_namidi.sequence tickByLocation:location];
    
    // TODO move measure
}

@end

@implementation MeasureView

- (void)awakeFromNib
{
    _grayColor = [Color gray].CGColor;
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

    [self drawMeasure:dirtyRect context:ctx];
    [self drawBottomLine:dirtyRect context:ctx];
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

- (void)drawBottomLine:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 1.0);
    CGContextSetStrokeColorWithColor(ctx, _grayColor);
    
    CGContextMoveToPoint(ctx, 0, BOTTOM_LINE_Y);
    CGContextAddLineToPoint(ctx, self.bounds.size.width, BOTTOM_LINE_Y);
    CGContextStrokePath(ctx);
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

@end
