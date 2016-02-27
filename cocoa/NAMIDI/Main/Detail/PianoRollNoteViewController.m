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

#define EVENT_RADIUS 3.0

@interface PianoRollNoteView : NSView {
    CGColorRef _gridColor;
    CGColorRef _gridWeakColor;
    CGColorRef _eventBorderColor[16];
    CGColorRef _eventFillColor[16];
}

@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (strong, nonatomic) SequenceRepresentation *sequence;
@property (strong, nonatomic) PlayerRepresentation *player;
@property (assign, nonatomic) NSColor *baseColor;

- (CGRect)noteRect:(NoteEvent *)note pixelPerTick:(CGFloat)pixelPerTick measureOffset:(CGFloat)measureOffset;
- (CGFloat)noteY:(int)noteNo;
@end

@interface PianoRollNoteViewController () <NAMidiRepresentationObserver, PlayerRepresentationObserver>
@property (strong, nonatomic) IBOutlet PianoRollNoteView *noteView;
@end

@implementation PianoRollNoteViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _noteView.scaleAssistant = _scaleAssistant;
    _noteView.trackSelection = _trackSelection;
    _noteView.player = _namidi.player;
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    _noteView.sequence = _namidi.sequence;
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
    [_trackSelection addObserver:self forKeyPath:@"selectionFlags" options:0 context:NULL];
    [_namidi addObserver:self];
    [_namidi.player addObserver:self];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    [self adjustVerticalScrollPosition];
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
        [_noteView layout];
    }
    else if (object == _trackSelection) {
        _noteView.needsDisplay = YES;
    }
}

- (void)adjustVerticalScrollPosition
{
    int low = 127;
    int high = 0;
    
    for (ChannelRepresentation *channel in _namidi.sequence.channels) {
        if ([_trackSelection isTrackSelected:channel.number]) {
            low = MIN(low, channel.noteRange.low);
            high = MAX(high, channel.noteRange.high);
        }
    }
    
    CGFloat centerY = [_noteView noteY:(low + high) / 2];
    NSClipView *clipView = (NSClipView *)self.view.superview;
    CGFloat halfHeight = clipView.frame.size.height / 2.0;
    [clipView scrollToPoint:CGPointMake(clipView.bounds.origin.x, roundf(centerY - halfHeight))];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    _noteView.sequence = sequence;
}

#pragma mark PlayerRepresentationObserver

- (void)player:(PlayerRepresentation *)player onSendNoteOn:(NoteEvent *)event
{
    CGRect rect = [_noteView noteRect:event pixelPerTick:_scaleAssistant.pixelPerTick measureOffset:_scaleAssistant.measureOffset];
    [_noteView setNeedsDisplayInRect:CGRectInset(rect, -20, 0)];
}

- (void)player:(PlayerRepresentation *)player onSendNoteOff:(NoteEvent *)event
{
    CGRect rect = [_noteView noteRect:event pixelPerTick:_scaleAssistant.pixelPerTick measureOffset:_scaleAssistant.measureOffset];
    [_noteView setNeedsDisplayInRect:CGRectInset(rect, -20, 0)];
}

@end

@implementation PianoRollNoteView

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
    
    int octaveFrom = MAX(-2, floor(dirtyRect.origin.y / PianoRollLayoutOctaveHeight) - 2);
    int octaveTo = MIN(8, ceil((dirtyRect.origin.y + dirtyRect.size.height) / PianoRollLayoutOctaveHeight));
    
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
    
    CGFloat pixelPerTick = _scaleAssistant.pixelPerTick;
    CGFloat measureOffset = _scaleAssistant.measureOffset;
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
            CGRect rect = [self noteRect:note pixelPerTick:pixelPerTick measureOffset:measureOffset];
            
            if (CGRectIntersectsRect(dirtyRect, rect)) {
                if (isPlaying && CGRectContainsPoint(rect, CGPointMake(currentX, CGRectGetMidY(rect)))) {
                    CGContextSetFillColorWithColor(ctx, _eventBorderColor[colorIndex]);
                }
                else {
                    CGContextSetFillColorWithColor(ctx, _eventFillColor[colorIndex]);
                }
                
                CGContextSetStrokeColorWithColor(ctx, _eventBorderColor[colorIndex]);
                [self drawRoundedRect:rect constext:ctx rarius:EVENT_RADIUS fill:YES stroke:YES];
            }
        }
    }
    
    CGContextRestoreGState(ctx);
}

- (CGRect)noteRect:(NoteEvent *)note pixelPerTick:(CGFloat)pixelPerTick measureOffset:(CGFloat)measureOffset
{
    CGFloat x = round(note->tick * pixelPerTick) + measureOffset;
    CGFloat y = [self noteY:note->noteNo];
    CGFloat width = round(note->gatetime * pixelPerTick);
    
    return CGRectMake(x, y - EVENT_RADIUS, width, EVENT_RADIUS * 2);
}

- (CGFloat)noteY:(int)noteNo
{
    int octave = noteNo / 12;
    int noteNoInOctave = noteNo % 12;
    
    return PianoRollLayoutNoteYInOctave[noteNoInOctave]
         + PianoRollLayoutOctaveHeight * octave
         + PianoRollLayoutNoteHeight + 0.5;
}

@end
