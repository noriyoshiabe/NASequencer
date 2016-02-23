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

@interface PianoRollKeyboardView : NSView {
    CGColorRef _edgeColor;
    CGColorRef _blackKeyColor;
    NSDictionary *_labelAttrs;
    CGFloat _textHeight;
}

@end

@interface PianoRollKeyboardViewController ()
@property (strong) IBOutlet PianoRollKeyboardView *keyboardView;
@end

@implementation PianoRollKeyboardViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
}

@end

@implementation PianoRollKeyboardView

- (void)awakeFromNib
{
    _edgeColor = [Color semiLightGray].CGColor;
    _blackKeyColor = [NSColor blackColor].CGColor;
    
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    ps.alignment = NSRightTextAlignment;
    
    _labelAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:10.0],
                    NSForegroundColorAttributeName: [Color gray],
                    NSParagraphStyleAttributeName: ps
                    };
    _textHeight = [@"C" sizeWithAttributes:_labelAttrs].height;
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
    [self drawLabel:dirtyRect context:ctx];
}

- (void)drawKeyboard:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 0.5);
    CGContextSetStrokeColorWithColor(ctx, _edgeColor);
    CGContextSetFillColorWithColor(ctx, _blackKeyColor);
    
    int octaveFrom = MAX(-2, floor(dirtyRect.origin.y / PianoRollLayoutOctaveHeight) - 2);
    int octaveTo = MIN(8, ceil((dirtyRect.origin.y + dirtyRect.size.height) / PianoRollLayoutOctaveHeight));
    
    for (int i = octaveFrom; i <= octaveTo; ++i) {
        for (int j = 0; j < 8; ++j) {
            CGFloat y = PianoRollLayoutOctaveHeight * (i + 2)
                      + PianoRollLayoutNoteHeight * 2 * j + 0.5;
            
            CGContextMoveToPoint(ctx, 0, y);
            CGContextAddLineToPoint(ctx, self.bounds.size.width, y);
            CGContextStrokePath(ctx);
        }
    }
    
    CGFloat blackKeyWidth = self.bounds.size.width / 2.0;
    
    for (int i = octaveFrom; i <= octaveTo; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (i == 8 && j == 3) {
                break;
            }
            
            CGFloat y = PianoRollLayoutOctaveHeight * (i + 2) + 0.5;
            
            switch (j) {
                case 0:
                    y += PianoRollLayoutNoteHeight * 1;
                    break;
                case 1:
                    y += PianoRollLayoutNoteHeight * 3;
                    break;
                case 2:
                    y += PianoRollLayoutNoteHeight * 7;
                    break;
                case 3:
                    y += PianoRollLayoutNoteHeight * 9;
                    break;
                case 4:
                    y += PianoRollLayoutNoteHeight * 11;
                    break;
            }
            
            CGRect rect = CGRectMake(0, y + PianoRollLayoutNoteHeight / 2.0, blackKeyWidth, PianoRollLayoutNoteHeight);
            CGContextAddRect(ctx, rect);
            CGContextFillPath(ctx);
            CGContextAddRect(ctx, rect);
            CGContextStrokePath(ctx);
        }
    }
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

- (void)drawLabel:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    int octaveFrom = MAX(-2, floor(dirtyRect.origin.y / PianoRollLayoutOctaveHeight) - 2);
    int octaveTo = MIN(8, ceil((dirtyRect.origin.y + dirtyRect.size.height) / PianoRollLayoutOctaveHeight));
 
    CGFloat x = self.bounds.size.width / 3.0 * 2.0 - 3.0;
    CGFloat width = self.bounds.size.width / 3.0;
    
    for (int i = octaveFrom; i <= octaveTo; ++i) {
        CGFloat y = PianoRollLayoutOctaveHeight * (i + 2) + PianoRollLayoutNoteHeight;
        CGRect rect = CGRectMake(x, y - _textHeight / 2.0, width, _textHeight);
        NSString *label = [NSString stringWithFormat:@"C%d", i];
        [label drawInRect:rect withAttributes:_labelAttrs];
    }
    
    CGContextRestoreGState(ctx);
}

@end
