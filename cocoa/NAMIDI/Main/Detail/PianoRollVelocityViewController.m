//
//  PianoRollVelocityViewController.m
//  NAMIDI
//
//  Created by abechan on 2/24/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollVelocityViewController.h"
#import "Color.h"

#define VELOCITY_BAR_WIDTH 3.0

@interface PianoRollVelocityView : NSView {
    CGColorRef _gridColor;
    CGColorRef _gridWeakColor;
    CGColorRef _eventBorderColor[16];
    CGColorRef _eventFillColor[16];
}

@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@property (strong, nonatomic) PlayerRepresentation *player;

- (CGRect)noteRect:(NoteEvent *)note pixelPerTick:(CGFloat)pixelPerTick measureOffset:(CGFloat)measureOffset viewHeight:(CGFloat)viewHeight;
@end

@interface PianoRollVelocityViewController () <NAMidiRepresentationObserver, PlayerRepresentationObserver>
@property (strong) IBOutlet PianoRollVelocityView *velocityView;
@end

@implementation PianoRollVelocityViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _velocityView.scaleAssistant = _scaleAssistant;
    _velocityView.trackSelection = _trackSelection;
    _velocityView.player = _namidi.player;
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    _velocityView.sequence = _namidi.sequence;
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
    [_trackSelection addObserver:self forKeyPath:@"selectionFlags" options:0 context:NULL];
    [_namidi addObserver:self];
    [_namidi.player addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    [_scaleAssistant removeObserver:self forKeyPath:@"scale"];
    [_trackSelection removeObserver:self forKeyPath:@"selectionFlags"];
    [_namidi removeObserver:self];
    [_namidi.player removeObserver:self];
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
        [_velocityView layout];
    }
    else if (object == _trackSelection) {
        _velocityView.needsDisplay = YES;
    }
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    _velocityView.sequence = sequence;
}

#pragma mark PlayerRepresentationObserver

- (void)player:(PlayerRepresentation *)player onSendNoteOn:(NoteEvent *)event
{
    CGRect rect = [_velocityView noteRect:event pixelPerTick:_scaleAssistant.pixelPerTick measureOffset:_scaleAssistant.measureOffset viewHeight:self.view.bounds.size.height];
    [_velocityView setNeedsDisplayInRect:CGRectInset(rect, -20, 0)];
}

- (void)player:(PlayerRepresentation *)player onSendNoteOff:(NoteEvent *)event
{
    CGRect rect = [_velocityView noteRect:event pixelPerTick:_scaleAssistant.pixelPerTick measureOffset:_scaleAssistant.measureOffset viewHeight:self.view.bounds.size.height];
    [_velocityView setNeedsDisplayInRect:CGRectInset(rect, -20, 0)];
}

@end

@implementation PianoRollVelocityView

- (void)awakeFromNib
{
    _gridColor = [Color grid].CGColor;
    _gridWeakColor = [Color gridWeak].CGColor;
    
    for (int i = 0; i < 16; ++i) {
        NSColor *baseColor = [Color channelColor:i + 1];
        _eventBorderColor[i] = [NSColor colorWithHue:baseColor.hueComponent saturation:0.64 brightness:0.89 alpha:1.0].CGColor;
        _eventFillColor[i] = [NSColor colorWithHue:baseColor.hueComponent saturation:0.64 brightness:0.89 alpha:0.3].CGColor;
        
        CGColorRetain(_eventBorderColor[i]);
        CGColorRetain(_eventFillColor[i]);
    }
}

- (void)dealloc
{
    if (_eventBorderColor[0]) {
        for (int i = 0; i < 16; ++i) {
            CGColorRelease(_eventBorderColor[i]);
            CGColorRelease(_eventFillColor[i]);
        }
    }
}

- (BOOL)isFlipped
{
    return NO;
}

- (void)setSequence:(SequenceRepresentation *)sequence
{
    _sequence = sequence;
    
    self.needsLayout = YES;
    self.needsDisplay = YES;
}

- (void)layout
{
    self.frame = CGRectMake(0, 0, _scaleAssistant.viewWidth, self.bounds.size.height);
    [super layout];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    if (_sequence) {
        CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
        [self drawGrid:dirtyRect context:ctx];
        [self drawEvents:dirtyRect context:ctx];
    }
}

- (void)drawGrid:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 0.5);
    
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
    CGFloat viewHeight = self.bounds.size.height;
    BOOL isPlaying = _player.isPlaying;
    CGFloat currentX = round(_player.tick * pixelPerTick) + measureOffset;
    
    CGContextSetLineWidth(ctx, 1.0);
    
    for (ChannelRepresentation *channel in _sequence.channels) {
        if (![_trackSelection isTrackSelected:channel.number]) {
            continue;
        }
        
        int colorIndex = channel.number - 1;
        
        for (MidiEventRepresentation *event in channel.events) {
            MidiEvent *raw = event.raw;
            if (MidiEventTypeNote != raw->type) {
                continue;
            }
            
            NoteEvent *note = (NoteEvent *)raw;
            CGRect rect = [self velocityRect:note pixelPerTick:pixelPerTick measureOffset:measureOffset viewHeight:viewHeight];
            
            if (CGRectIntersectsRect(dirtyRect, rect)) {
                if (isPlaying) {
                    CGRect rect = [self noteRect:note pixelPerTick:pixelPerTick measureOffset:measureOffset viewHeight:viewHeight];
                    if (CGRectContainsPoint(rect, CGPointMake(currentX, CGRectGetMidY(rect)))) {
                        CGContextSetFillColorWithColor(ctx, _eventBorderColor[colorIndex]);
                    }
                    else {
                        CGContextSetFillColorWithColor(ctx, _eventFillColor[colorIndex]);
                    }
                }
                else {
                    CGContextSetFillColorWithColor(ctx, _eventFillColor[colorIndex]);
                }
                
                CGContextSetStrokeColorWithColor(ctx, _eventBorderColor[colorIndex]);
                CGContextAddRect(ctx, rect);
                CGContextFillPath(ctx);
                CGContextAddRect(ctx, rect);
                CGContextStrokePath(ctx);
            }
        }
    }
    
    CGContextRestoreGState(ctx);
}

- (CGRect)velocityRect:(NoteEvent *)note pixelPerTick:(CGFloat)pixelPerTick measureOffset:(CGFloat)measureOffset viewHeight:(CGFloat)viewHeight
{
    CGFloat x = round(note->tick * pixelPerTick) + measureOffset;
    CGFloat height = viewHeight * ((CGFloat)note->velocity / 127.0);
    return CGRectMake(x + 0.5, 0, VELOCITY_BAR_WIDTH, height);
}

- (CGRect)noteRect:(NoteEvent *)note pixelPerTick:(CGFloat)pixelPerTick measureOffset:(CGFloat)measureOffset viewHeight:(CGFloat)viewHeight
{
    CGFloat x = round(note->tick * pixelPerTick) + measureOffset;
    CGFloat width = round(note->gatetime * pixelPerTick);
    return CGRectMake(x, 0, width, viewHeight);
}

@end