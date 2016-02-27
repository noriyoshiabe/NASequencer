//
//  PianoRollKeyboardViewController.m
//  NAMIDI
//
//  Created by abechan on 2/24/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollKeyboardViewController.h"
#import "Color.h"
#import "PianoRollLayout.h"

typedef struct _Key {
    CGRect rect;
    int noteNo;
    BOOL isPressed;
} Key;

typedef struct _Octave {
    Key whiteKey[7];
    Key blackKey[5];
} Octave;

#define WHITE_KEY_WIDTH 60.0
#define BLACK_KEY_WIDTH 30.0

#define WHITE_KEY_RECT(y) CGRectMake(0, y, WHITE_KEY_WIDTH, PianoRollLayoutNoteHeight * 2);
#define BLACK_KEY_RECT(y) CGRectMake(0, y, BLACK_KEY_WIDTH, PianoRollLayoutNoteHeight);

#define WHITE_KEY_LABEL_X 37.0
#define WHITE_KEY_LABEL_WIDTH 20.0

@interface PianoRollKeyboardView : NSView {
    CGColorRef _edgeColor;
    CGColorRef _blackKeyColor;
    CGColorRef _hitKeyColor;
    NSDictionary *_labelAttrs;
    CGFloat _textHeight;

@public
    Octave _octaves[11];
}

@end

@interface PianoRollKeyboardViewController () {
    Key *_pressedKey;
}

@property (strong) IBOutlet PianoRollKeyboardView *keyboardView;
@end

@implementation PianoRollKeyboardViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    CGPoint point = [self.view convertPoint:theEvent.locationInWindow fromView:self.view.window.contentView];
    [self playKeyInPoint:point];
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (_pressedKey) {
        [self sendNoteOff:_pressedKey];
        _pressedKey->isPressed = NO;
        [_keyboardView setNeedsDisplayInRect:_pressedKey->rect];
        _pressedKey = NULL;
    }
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    CGPoint point = [self.view convertPoint:theEvent.locationInWindow fromView:self.view.window.contentView];
    
    if (_pressedKey) {
        if (!CGRectContainsPoint(_pressedKey->rect, point)) {
            [self sendNoteOff:_pressedKey];
            _pressedKey->isPressed = NO;
            [_keyboardView setNeedsDisplayInRect:_pressedKey->rect];
            _pressedKey = NULL;
            
            [self playKeyInPoint:point];
        }
    }
    else {
        [self playKeyInPoint:point];
    }
}

- (void)playKeyInPoint:(CGPoint)point
{
    int octaveIndex = MIN(10, MAX(0, floor(point.y / PianoRollLayoutOctaveHeight)));
    Octave *octave = &_keyboardView->_octaves[octaveIndex];
    
    for (int i = 0; i < 5; ++i) {
        if (CGRectContainsPoint(octave->blackKey[i].rect, point)) {
            _pressedKey = &octave->blackKey[i];
            goto HIT;
        }
    }
    
    for (int i = 0; i < 7; ++i) {
        if (CGRectContainsPoint(octave->whiteKey[i].rect, point)) {
            _pressedKey = &octave->whiteKey[i];
            goto HIT;
        }
    }
    
    return;
    
HIT:
    [self sendNoteOn:_pressedKey];
    _pressedKey->isPressed = YES;
    [_keyboardView setNeedsDisplayInRect:_pressedKey->rect];
}

- (void)sendNoteOn:(Key *)key
{
    for (int i = 1; i <= 16; ++i) {
        if ([_trackSelection isTrackSelected:i]) {
            NoteEvent note;
            note.channel = i;
            note.noteNo = key->noteNo;
            note.velocity = 100;
            [_namidi.mixer sendNoteOn:&note];
        }
    }
}

- (void)sendNoteOff:(Key *)key
{
    for (int i = 1; i <= 16; ++i) {
        if ([_trackSelection isTrackSelected:i]) {
            NoteEvent note;
            note.channel = i;
            note.noteNo = _pressedKey->noteNo;
            [_namidi.mixer sendNoteOff:&note];
        }
    }
}

@end

@implementation PianoRollKeyboardView

- (void)awakeFromNib
{
    _edgeColor = [Color semiLightGray].CGColor;
    _blackKeyColor = [NSColor blackColor].CGColor;
    _hitKeyColor = [NSColor colorWith8bitRed:160.0 green:160.0 blue:255.0 alpha:255.0].CGColor;
    CGColorRetain(_hitKeyColor);
    
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    ps.alignment = NSRightTextAlignment;
    
    _labelAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:10.0],
                    NSForegroundColorAttributeName: [Color gray],
                    NSParagraphStyleAttributeName: ps
                    };
    _textHeight = [@"C" sizeWithAttributes:_labelAttrs].height;
    
    [self initializeKeys];
}

- (void)dealloc
{
    CGColorRelease(_hitKeyColor);
}

- (void)initializeKeys
{
    for (int i = 0; i < 11; ++i) {
        Octave *octave = &_octaves[i];
        int noteShift = i * 12;
        CGFloat offsetY = i * PianoRollLayoutOctaveHeight + 0.5;
        
        octave->whiteKey[0].noteNo = noteShift + 0;
        octave->whiteKey[1].noteNo = noteShift + 2;
        octave->whiteKey[2].noteNo = noteShift + 4;
        octave->whiteKey[3].noteNo = noteShift + 5;
        octave->whiteKey[4].noteNo = noteShift + 7;
        octave->whiteKey[5].noteNo = noteShift + 9;
        octave->whiteKey[6].noteNo = noteShift + 11;
        
        octave->blackKey[0].noteNo = noteShift + 1;
        octave->blackKey[1].noteNo = noteShift + 3;
        octave->blackKey[2].noteNo = noteShift + 6;
        octave->blackKey[3].noteNo = noteShift + 8;
        octave->blackKey[4].noteNo = noteShift + 10;
        
        octave->whiteKey[0].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 0);
        octave->whiteKey[1].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 2);
        octave->whiteKey[2].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 4);
        octave->whiteKey[3].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 6);
        octave->whiteKey[4].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 8);
        octave->whiteKey[5].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 10);
        octave->whiteKey[6].rect = WHITE_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 12);
        
        octave->blackKey[0].rect = BLACK_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 1 + PianoRollLayoutNoteHeight / 2.0);
        octave->blackKey[1].rect = BLACK_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 3 + PianoRollLayoutNoteHeight / 2.0);
        octave->blackKey[2].rect = BLACK_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 7 + PianoRollLayoutNoteHeight / 2.0);
        octave->blackKey[3].rect = BLACK_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 9 + PianoRollLayoutNoteHeight / 2.0);
        octave->blackKey[4].rect = BLACK_KEY_RECT(offsetY + PianoRollLayoutNoteHeight * 11 + PianoRollLayoutNoteHeight / 2.0);
    }
}

- (BOOL)isFlipped
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    [self drawKeyboard:dirtyRect context:ctx];
}

- (void)drawKeyboard:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    int octaveFrom = MAX(-2, floor(dirtyRect.origin.y / PianoRollLayoutOctaveHeight) - 2);
    int octaveTo = MIN(8, ceil((dirtyRect.origin.y + dirtyRect.size.height) / PianoRollLayoutOctaveHeight));
    
    for (int i = octaveFrom; i <= octaveTo; ++i) {
        int whiteKeyCount = i == 8 ? 5 : 7;
        int blackKeyCount = i == 8 ? 3 : 5;
        Octave *octave = &_octaves[i + 2];
        
        CGContextSetLineWidth(ctx, 0.5);
        CGContextSetStrokeColorWithColor(ctx, _edgeColor);
        CGContextSetFillColorWithColor(ctx, _hitKeyColor);
        
        for (int j = 0; j < whiteKeyCount; ++j) {
            if (octave->whiteKey[j].isPressed) {
                CGContextAddRect(ctx, octave->whiteKey[j].rect);
                CGContextFillPath(ctx);
            }
            
            CGContextAddRect(ctx, octave->whiteKey[j].rect);
            CGContextStrokePath(ctx);
        }
        
        for (int j = 0; j < blackKeyCount; ++j) {
            CGColorRef color = octave->blackKey[j].isPressed ? _hitKeyColor : _blackKeyColor;
            CGContextSetFillColorWithColor(ctx, color);
            CGContextAddRect(ctx, octave->blackKey[j].rect);
            CGContextFillPath(ctx);
            CGContextAddRect(ctx, octave->blackKey[j].rect);
            CGContextStrokePath(ctx);
        }
        
        CGFloat y = PianoRollLayoutOctaveHeight * (i + 2) + PianoRollLayoutNoteHeight;
        CGRect rect = CGRectMake(WHITE_KEY_LABEL_X, y - _textHeight / 2.0, WHITE_KEY_LABEL_WIDTH, _textHeight);
        NSString *label = [NSString stringWithFormat:@"C%d", i];
        [label drawInRect:rect withAttributes:_labelAttrs];
    }
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

@end
