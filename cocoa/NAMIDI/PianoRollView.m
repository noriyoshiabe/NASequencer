//
//  PianoRollView.m
//  NAMIDI
//
//  Created by 張阿部 on 2015/03/03.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#import "PianoRollView.h"
#import <CoreGraphics/CoreGraphics.h>

#define WIDTH_PER_TICK (50.0 / 480.0)
#define HEIGHT_PER_KEY (10.0)

@interface PianoRollView() {
    CGColorRef gridColorKey;
    CGColorRef gridColorCKey;
    CGColorRef gridColorBeat;
    CGColorRef gridColorMeasure;
    CGColorRef *noteColors;
    CGColorRef *noteBorderColors;
    CGColorRef currentPositionColor;
 
    ParseContext *parseContext;
    PlayerContext *playerContext;
    
    CGLayerRef gridLayer;
    CGLayerRef notesLayer;
    CGLayerRef positionLayer;
    CGLayerRef playingLayer;
    
    CFAbsoluteTime time;
    int drawCount;
}

@end


@implementation PianoRollView

- (void)awakeFromNib
{
    gridColorKey = CGColorCreateGenericRGB(0.2, 0.5, 0.9, 0.1);
    gridColorCKey = CGColorCreateGenericRGB(0.2, 0.5, 0.9, 0.3);
    gridColorBeat = CGColorCreateGenericRGB(0.2, 0.5, 0.9, 0.1);
    gridColorMeasure = CGColorCreateGenericRGB(0.2, 0.5, 0.9, 0.3);
    
    noteColors = calloc(16, sizeof(CGColorRef));
    for (int i = 0; i < 16; ++i) {
        @autoreleasepool {
            noteColors[i] = CGColorCreateCopy([NSColor colorWithHue:1.0 / 15.0 * i saturation:1.0 brightness:1.0 alpha:0.5].CGColor);
        }
    }
    
    noteBorderColors = calloc(16, sizeof(CGColorRef));
    for (int i = 0; i < 16; ++i) {
        @autoreleasepool {
            noteBorderColors[i] = CGColorCreateCopy([NSColor colorWithHue:1.0 / 15.0 * i saturation:1.0 brightness:1.0 alpha:1.0].CGColor);
        }
    }
    
    currentPositionColor = CGColorCreateGenericRGB(0.75, 0.75, 0.0, 1.0);
    
    time = CFAbsoluteTimeGetCurrent();
    drawCount = 0;
}

- (void)dealloc
{
    CGColorRelease(gridColorKey);
    CGColorRelease(gridColorCKey);
    CGColorRelease(gridColorBeat);
    CGColorRelease(gridColorMeasure);
    
    for (int i = 0; i < 16; ++i) {
        CGColorRelease(noteColors[i]);
        CGColorRelease(noteBorderColors[i]);
    }
    
    CGColorRelease(currentPositionColor);
    
    free(noteColors);
    free(noteBorderColors);
    
    if (gridLayer) CGLayerRelease(gridLayer);
    if (notesLayer) CGLayerRelease(notesLayer);
    if (positionLayer) CGLayerRelease(positionLayer);
    if (playingLayer) CGLayerRelease(playingLayer);
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
    CGContextRef context = [NSGraphicsContext currentContext].graphicsPort;
    CGContextClipToRect(context, dirtyRect);
    
    if (gridLayer) {
        CGContextDrawLayerAtPoint(context, CGPointZero, gridLayer);
    }
    
    if (notesLayer) {
        CGContextDrawLayerAtPoint(context, CGPointZero, notesLayer);
    }
    
    if (playerContext) {
        [self drawPlayingLayer:context dirtyRect:dirtyRect];
    }
    
    if (positionLayer) {
        int32_t tick = playerContext ? playerContext->tick : 0;
        
        CGContextSaveGState(context);
        CGContextTranslateCTM(context, (tick + 120.0) * WIDTH_PER_TICK, 0);
        CGContextDrawLayerAtPoint(context, CGPointZero, positionLayer);
        CGContextRestoreGState(context);
    }
    
    [self calcFPS];
}

- (void)onParseFinished:(NAMidiProxy *)namidi context:(ParseContext *)context
{
    if (context->error) {
        return;
    }
    
    parseContext = context;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        CGFloat width = (parseContext->sequence->length + 240) * WIDTH_PER_TICK;
        self.frame = CGRectMake(0, 0, roundf(width), roundf((127 + 2) * HEIGHT_PER_KEY));
        
        CGContextRef context = [NSGraphicsContext currentContext].graphicsPort;
        
        gridLayer = CGLayerCreateWithContext(context, self.bounds.size, nil);
        notesLayer = CGLayerCreateWithContext(context, self.bounds.size, nil);
        positionLayer = CGLayerCreateWithContext(context, CGSizeMake(1, self.bounds.size.height), nil);
        playingLayer = CGLayerCreateWithContext(context, self.bounds.size, nil);
        
        [self drawGridLayer];
        [self drawNotesLayer];
        [self drawPositionLayer];
        
        NSView *parent = self.superview.superview;
        [self setNeedsDisplayInRect:[parent convertRect:parent.bounds toView: self]];
    });
}

- (void)onPlayerContextChanged:(NAMidiProxy *)namidi context:(PlayerContext *)context
{
    playerContext = context;
    dispatch_async(dispatch_get_main_queue(), ^{
        NSView *parent = self.superview.superview;
        [self setNeedsDisplayInRect:[parent convertRect:parent.bounds toView: self]];
    });
}

- (void)drawGridLayer
{
    CGContextRef ctx = CGLayerGetContext(gridLayer);
    CGContextSetLineWidth(ctx, 1.0);
    
    for (int i = 0; i < 128; ++i) {
        CGFloat y = HEIGHT_PER_KEY * (i + 1);
        CGColorRef color = 0 == (127 - i) % 12 ? gridColorCKey : gridColorKey;
        CGContextSetStrokeColorWithColor(ctx, color);
        CGContextMoveToPoint(ctx, 0, y);
        CGContextAddLineToPoint(ctx, self.bounds.size.width, y);
        CGContextStrokePath(ctx);
    }
    
    int32_t tick = 0;
    int32_t tickTo = self.bounds.size.width / WIDTH_PER_TICK;

    Location location = TimeTableTick2Location(parseContext->sequence->timeTable, tick);
    location.t = 0;
    
    int16_t numerator;
    int16_t denominator;
    
    TimeTableGetTimeSignByTick(parseContext->sequence->timeTable, tick, &numerator, &denominator);
    
    while (tick <= tickTo) {
        CGColorRef color = 1 == location.b ? gridColorMeasure : gridColorBeat;
        CGFloat x = roundf((tick + 120) * WIDTH_PER_TICK);
        
        CGContextSetStrokeColorWithColor(ctx, color);
        CGContextMoveToPoint(ctx, x, 0);
        CGContextAddLineToPoint(ctx, x, self.bounds.size.height);
        CGContextStrokePath(ctx);
        
        if (numerator < ++location.b) {
            location.b = 1;
            ++location.m;
            
            TimeTableGetTimeSignByTick(parseContext->sequence->timeTable, tick, &numerator, &denominator);
        }
        
        tick = TimeTableLocation2Tick(parseContext->sequence->timeTable, location.m, location.b, 0);
    }
}

- (void)drawNotesLayer
{
    CGContextRef ctx = CGLayerGetContext(notesLayer);
    
    for (CFIndex i = 0, size = CFArrayGetCount(parseContext->sequence->events); i < size; ++i) {
        const MidiEvent *event = CFArrayGetValueAtIndex(parseContext->sequence->events, i);
        if (NATypeOf(event, NoteEvent)) {
            [self drawNote:ctx note:(NoteEvent *)event active:NO];
        }
    }
}

- (void)drawNote:(CGContextRef)ctx note:(const NoteEvent *)note active:(BOOL)active
{
    CGFloat left = roundf((note->__.tick + 120) * WIDTH_PER_TICK);
    CGFloat right = roundf((note->__.tick + 120 + note->gatetime) * WIDTH_PER_TICK);
    CGFloat y = roundf((127 - note->noteNo + 1) * HEIGHT_PER_KEY);
    
    CGRect rect = CGRectMake(left - 5, y - 5, right - left, 10);
    
    CGFloat lx = CGRectGetMinX(rect);
    CGFloat cx = CGRectGetMidX(rect);
    CGFloat rx = CGRectGetMaxX(rect);
    CGFloat by = CGRectGetMinY(rect);
    CGFloat cy = CGRectGetMidY(rect);
    CGFloat ty = CGRectGetMaxY(rect);
    CGFloat radius = 5.0;
    
    CGContextSetLineWidth(ctx, 1.0);
    
    if (active) {
        CGContextMoveToPoint(ctx, lx, cy);
        CGContextAddArcToPoint(ctx, lx, by, cx, by, radius);
        CGContextAddArcToPoint(ctx, rx, by, rx, cy, radius);
        CGContextAddArcToPoint(ctx, rx, ty, cx, ty, radius);
        CGContextAddArcToPoint(ctx, lx, ty, lx, cy, radius);
        CGContextClosePath(ctx);
        
        CGContextSetFillColorWithColor(ctx, noteBorderColors[note->channel - 1]);
        CGContextFillPath(ctx);
    }
    else {
        CGContextMoveToPoint(ctx, lx, cy);
        CGContextAddArcToPoint(ctx, lx, by, cx, by, radius);
        CGContextAddArcToPoint(ctx, rx, by, rx, cy, radius);
        CGContextAddArcToPoint(ctx, rx, ty, cx, ty, radius);
        CGContextAddArcToPoint(ctx, lx, ty, lx, cy, radius);
        CGContextClosePath(ctx);
        
        CGContextSetFillColorWithColor(ctx, noteColors[note->channel - 1]);
        CGContextFillPath(ctx);
        
        CGContextMoveToPoint(ctx, lx, cy);
        CGContextAddArcToPoint(ctx, lx, by, cx, by, radius);
        CGContextAddArcToPoint(ctx, rx, by, rx, cy, radius);
        CGContextAddArcToPoint(ctx, rx, ty, cx, ty, radius);
        CGContextAddArcToPoint(ctx, lx, ty, lx, cy, radius);
        CGContextClosePath(ctx);
        
        CGContextSetStrokeColorWithColor(ctx, noteBorderColors[note->channel - 1]);
        CGContextStrokePath(ctx);
    }
}

- (void)drawPositionLayer
{
    CGContextRef ctx = CGLayerGetContext(positionLayer);
        
    CGContextSetLineWidth(ctx, 1.0);
    
    CGContextSetStrokeColorWithColor(ctx, currentPositionColor);
    CGContextMoveToPoint(ctx, 0, 0);
    CGContextAddLineToPoint(ctx, 0, self.bounds.size.height);
    CGContextStrokePath(ctx);
}

- (void)drawPlayingLayer:(CGContextRef)ctx dirtyRect:(NSRect)dirtyRect
{
    if (PLAYER_STATE_PLAYING != playerContext->state) {
        return;
    }
    
    CGFloat playing = (self->playerContext->tick + 120) * WIDTH_PER_TICK + 0.5;
    
    for (CFIndex i = 0, size = CFArrayGetCount(playerContext->playing); i < size; ++i) {
        const NoteEvent *note = CFArrayGetValueAtIndex(playerContext->playing, i);
        if (self->playerContext->tick > note->__.tick + note->gatetime) {
            continue;
        }
        else if (self->playerContext->tick < note->__.tick) {
            break;
        }
        
        CGFloat left = roundf((note->__.tick + 120) * WIDTH_PER_TICK + 0.5);
        CGFloat right = roundf((note->__.tick + 120 + note->gatetime) * WIDTH_PER_TICK + 0.5);
        CGFloat y = roundf((127 - note->noteNo + 1) * WIDTH_PER_TICK + 0.5);
        CGRect rect = CGRectMake(left - 5, y - 5, right - left, 10);
        if (CGRectIntersectsRect(rect, CGRectMake(playing, 0, 0, self.bounds.size.height))) {
            [self drawNote:ctx note:note active: true];
        }
    }
}

- (void)calcFPS
{
    ++drawCount;
    
    CFAbsoluteTime _time = CFAbsoluteTimeGetCurrent();
    if (1.0 < _time - time) {
        printf("FPS: %d\n", drawCount);
        time = _time;
        drawCount = 0;
    }
}


@end