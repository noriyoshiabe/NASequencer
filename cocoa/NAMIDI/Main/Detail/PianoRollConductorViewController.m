//
//  PianoRollConductorViewController.m
//  NAMIDI
//
//  Created by abechan on 2/23/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollConductorViewController.h"
#import "Color.h"

@interface PianoRollConductorView : NSView {
    CGColorRef _gridColor;
    CGColorRef _gridWeakColor;
    NSDictionary *_eventTextAttrs;
    CGSize _textSize;
}

@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@end

@interface PianoRollConductorViewController ()
@property (strong, nonatomic) IBOutlet PianoRollConductorView *conductorView;
@end

@implementation PianoRollConductorViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _conductorView.scaleAssistant = _scaleAssistant;
    _conductorView.trackSelection = _trackSelection;
    _conductorView.sequence = _namidi.sequence;
    
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
        [_conductorView layout];
    }
}

@end

@implementation PianoRollConductorView

- (void)awakeFromNib
{
    _gridColor = [Color grid].CGColor;
    _gridWeakColor = [Color gridWeak].CGColor;
    _eventTextAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:8.0], NSForegroundColorAttributeName: [Color darkGray]};
    _textSize = [@"8" sizeWithAttributes:_eventTextAttrs];
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
    [self drawGrid:dirtyRect context:ctx];
    [self drawEvents:dirtyRect context:ctx];
}

- (void)drawGrid:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 0.5);
    CGContextSetStrokeColorWithColor(ctx, _gridWeakColor);
    
    CGFloat sixth = floor(self.bounds.size.height / 6);
    for (int i = 0; i < 3; ++i) {
        CGFloat y = sixth * i * 2 + sixth + 0.5;
        CGContextMoveToPoint(ctx, 0, y);
        CGContextAddLineToPoint(ctx, self.bounds.size.width, y);
        CGContextStrokePath(ctx);
    }
    
    CGFloat tickPerPixel = _scaleAssistant.tickPerPixel;
    CGFloat pixelPerTick = _scaleAssistant.pixelPerTick;
    CGFloat measureOffset = _scaleAssistant.measureOffset;
    CGFloat height = self.bounds.size.height;
    
    int tick = (dirtyRect.origin.x) * tickPerPixel;
    int tickTo = (dirtyRect.origin.x + dirtyRect.size.width) * tickPerPixel;
    
    Location location = [_sequence locationByTick:tick];
    location.t = 0;
    tick = [_sequence tickByLocation:location];
    TimeSign timeSign = [_sequence timeSignByTick:tick];
    
    while (tick <= tickTo && tick <= _sequence.length) {
        bool isMeasure = 1 == location.b;
        
        CGFloat x = round(tick * pixelPerTick) + measureOffset;
        CGColorRef color = isMeasure ? _gridColor : _gridWeakColor;
        
        CGContextSetStrokeColorWithColor(ctx, color);
        CGContextMoveToPoint(ctx, x, 0);
        CGContextAddLineToPoint(ctx, x, height);
        CGContextStrokePath(ctx);
        
        if (timeSign.numerator < ++location.b) {
            location.b = 1;
            ++location.m;
            timeSign = [_sequence timeSignByTick:tick];
        }
        
        tick = [_sequence tickByLocation:location];
    }
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

- (void)drawEvents:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGFloat pixelPerTick = _scaleAssistant.pixelPerTick;
    CGFloat measureOffset = _scaleAssistant.measureOffset;
    CGFloat sixth = floor(self.bounds.size.height / 6);
    
    for (MidiEventRepresentation *event in _sequence.eventsOfConductorTrack) {
        NSString *text = nil;
        MidiEvent *raw = event.raw;
        CGFloat y;
        
        switch (raw->type) {
            case MidiEventTypeTime:
            {
                TimeEvent *time = (TimeEvent *)raw;
                text = [NSString stringWithFormat:@"%d/%d", time->numerator, time->denominator];
                y = sixth + 0.5 - _textSize.height / 2.0;
                break;
            }
            case MidiEventTypeTempo:
            {
                TempoEvent *tempo = (TempoEvent *)raw;
                text = [NSString stringWithFormat:@"%.2f", tempo->tempo];
                y = sixth * 3 + 0.5 - _textSize.height / 2.0;
                break;
            }
            case MidiEventTypeMarker:
            {
                MarkerEvent *marker = (MarkerEvent *)raw;
                text = [NSString stringWithUTF8String:marker->text];
                y = sixth * 5 + 0.5 - _textSize.height / 2.0;
                break;
            }
            default:
                continue;
        }
        
        CGFloat x = round(raw->tick * pixelPerTick) + measureOffset - _textSize.width / 2.0;
        [text drawAtPoint:CGPointMake(x, y) withAttributes:_eventTextAttrs];
    }
    
    CGContextRestoreGState(ctx);
}

@end