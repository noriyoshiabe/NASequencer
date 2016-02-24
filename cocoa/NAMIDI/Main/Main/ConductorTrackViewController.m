//
//  ConductorTrackViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ConductorTrackViewController.h"
#import "Color.h"
#import "Stub.h"

#define EVENT_Y      9.5
#define EVENT_RADIUS 4.0

@interface ConductorTrackView : NSView {
    CGColorRef _whiteColor;
    CGColorRef _lightGrayColor;
    CGColorRef _blackColor;
}

@property (assign, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end

@interface ConductorTrackViewController ()
@property (strong, nonatomic) IBOutlet ConductorTrackView *conductorTrackView;
@end

@implementation ConductorTrackViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    _conductorTrackView.scaleAssistant = _scaleAssistant;
    _conductorTrackView.trackSelection = _trackSelection;
    _conductorTrackView.sequence = _namidi.sequence;
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
    [_trackSelection addObserver:self forKeyPath:@"selectionFlags" options:0 context:NULL];
}

- (void)dealloc
{
    [_scaleAssistant removeObserver:self forKeyPath:@"scale"];
    [_trackSelection removeObserver:self forKeyPath:@"selectionFlags"];
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
        [_conductorTrackView layout];
    }
    else if (object == _trackSelection) {
        if ([_trackSelection isTrackSelectionChanged:0]) {
            _conductorTrackView.needsDisplay = YES;
        }
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [_trackSelection click:0 event:theEvent];
}

@end

@implementation ConductorTrackView

- (void)awakeFromNib
{
    _whiteColor = [NSColor whiteColor].CGColor;
    _lightGrayColor = [Color lightGray].CGColor;
    _blackColor = [NSColor blackColor].CGColor;
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
    
    [self drawBackground:dirtyRect context:ctx];
    [self drawEvent:dirtyRect context:ctx];
}

- (void)drawBackground:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    if ([_trackSelection isTrackSelected:0]) {
        CGContextSaveGState(ctx);
        CGContextSetFillColorWithColor(ctx, _whiteColor);
        CGContextFillRect(ctx, CGRectMake(0, 0, self.bounds.size.width, self.bounds.size.height));
        CGContextClipToRect(ctx, dirtyRect);
        CGContextRestoreGState(ctx);
    }
}

- (void)drawEvent:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetFillColorWithColor(ctx, [_trackSelection isTrackSelected:0] ? _blackColor : _lightGrayColor);
    
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