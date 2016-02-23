//
//  PianoRollNoteViewController.m
//  NAMIDI
//
//  Created by abechan on 2/23/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollNoteViewController.h"
#import "Color.h"
#import "PianoRollLayout.h"

@interface PianoRollNoteView : NSView {
    CGColorRef _gridColor;
    CGColorRef _gridWeakColor;
    CGColorRef _eventBorderColor;
    CGColorRef _eventFillColor;
}

@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@end

@interface PianoRollNoteViewController ()
@property (strong, nonatomic) IBOutlet PianoRollNoteView *noteView;
@end

@implementation PianoRollNoteViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _noteView.scaleAssistant = _scaleAssistant;
    _noteView.trackSelection = _trackSelection;
    _noteView.sequence = _namidi.sequence;
    
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
        [_noteView layout];
    }
}

@end

@implementation PianoRollNoteView

- (void)awakeFromNib
{
    _gridColor = [Color grid].CGColor;
    _gridWeakColor = [Color gridWeak].CGColor;
    
    //_eventBorderColor = ;
    //_eventFillColor = ;
}

- (void)dealloc
{
    // TODO
}

- (BOOL)isFlipped
{
    return NO;
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
    
    int octaveFrom = MAX(-2, floor(dirtyRect.origin.y / PianoRollLayoutOctaveHeight) - 2);
    int octaveTo = MIN(8, octaveFrom + ceil(dirtyRect.size.height / PianoRollLayoutOctaveHeight));
    
    for (int i = octaveFrom; i <= octaveTo; ++i) {
        for (int j = 0; j < 12; ++j) {
            CGFloat y = PianoRollLayoutNoteYInOctave[j]
                      + PianoRollLayoutOctaveHeight * (i + 2)
                      + PianoRollLayoutNoteHeight + 0.5;
            
            CGColorRef color = 0 == j ? _gridColor : _gridWeakColor;
            CGContextSetStrokeColorWithColor(ctx, color);
            CGContextMoveToPoint(ctx, 0, y);
            CGContextAddLineToPoint(ctx, self.bounds.size.width, y);
            CGContextStrokePath(ctx);
        }
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
    
    // TODO
    
    CGContextRestoreGState(ctx);
}

@end
