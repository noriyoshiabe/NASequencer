//
//  EditorViewController.m
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

@import CoreText;

#import "EditorViewController.h"

@interface LineNumberView : NSRulerView {
    CGFontRef cgFont;
    CTFontRef ctFont;
}
@property (strong, nonatomic) NSFont *font;
- (instancetype)initWithTextView:(NSTextView *)textView;
@end

@interface EditorViewController ()
@end

@implementation EditorViewController

- (NSString *)nibName
{
    return @"EditorViewController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _textView.font = [NSFont fontWithName:@"Menlo" size:11];
    _textView.enclosingScrollView.verticalRulerView = [[LineNumberView alloc] initWithTextView:_textView];
    _textView.enclosingScrollView.hasVerticalRuler = YES;
    _textView.enclosingScrollView.rulersVisible = YES;
    _textView.automaticQuoteSubstitutionEnabled = NO;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(selectedRangeDidChange:) name: NSTextViewDidChangeSelectionNotification object:_textView];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(textDidChange:) name: NSTextDidChangeNotification object:_textView];
    
    _textView.string = [NSString stringWithContentsOfURL:_file.url encoding:NSUTF8StringEncoding error:nil];
    _textView.selectedRange = NSMakeRange(0, 0);
}

- (void)viewDidAppear
{
    [self.view.window makeFirstResponder:_textView];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)selectedRangeDidChange:(NSNotification *)notification
{
    if (0 == _textView.selectedRange.location) {
        [_delegate editorViewController:self didUpdateLine:1 column:1];
        return;
    }
    
    __block NSUInteger line = 0;
    __block NSUInteger column = 0;
    
    [_textView.string enumerateSubstringsInRange:NSMakeRange(0, _textView.selectedRange.location)
                                         options:NSStringEnumerationByLines | NSStringEnumerationSubstringNotRequired
                                      usingBlock:^(NSString * _Nullable substring, NSRange substringRange, NSRange enclosingRange, BOOL * _Nonnull stop) {
                                          ++line;
                                          column = substringRange.length == enclosingRange.length ? substringRange.length + 1 : 1;
                            }];
    
    
    
    if ([[NSCharacterSet newlineCharacterSet] characterIsMember:[_textView.string characterAtIndex:_textView.selectedRange.location - 1]]) {
        ++line;
    }
    
    [_delegate editorViewController:self didUpdateLine:line column:column];
}

- (void)textDidChange:(NSNotification *)notification
{
    NSTextStorage *storage = [_textView textStorage];
    NSMutableParagraphStyle *style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    [style setLineBreakMode:NSLineBreakByCharWrapping];
    [storage addAttribute:NSParagraphStyleAttributeName value:style range:NSMakeRange(0, [storage length])];
}

#pragma mark Menu Action

- (IBAction)performClose:(id)sender
{
    [_delegate editorViewController:self didPerformCloseAction:sender];
}

@end

#pragma mark Line Number

@implementation LineNumberView

- (instancetype)initWithTextView:(NSTextView *)textView
{
    self = [super initWithScrollView:textView.enclosingScrollView orientation:NSVerticalRuler];
    if (self) {
        self.font = textView.font;
        cgFont = CGFontCreateWithFontName((CFStringRef)[_font fontName]);
        ctFont = CTFontCreateWithGraphicsFont(cgFont, _font.pointSize, nil, nil);
        self.clientView = textView;
        
        textView.postsFrameChangedNotifications = true;
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(frameDidChange:) name: NSViewFrameDidChangeNotification object:textView];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(textDidChange:) name: NSTextDidChangeNotification object:textView];
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    CGFontRelease(cgFont);
    CFRelease(ctFont);
}

- (void)frameDidChange:(NSNotification *)notification
{
    self.needsDisplay = YES;
}

- (void)textDidChange:(NSNotification *)notification
{
    self.needsDisplay = YES;
}

- (void)drawHashMarksAndLabelsInRect:(NSRect)rect
{
    [[NSColor darkGrayColor] setFill];
    NSRectFill(rect);
    
    NSTextView *textView = (NSTextView *)self.clientView;
    NSLayoutManager *layoutManager = textView.layoutManager;
    NSString *string = textView.string;
    CGFloat width = self.ruleThickness;
    CGFloat padding = 5.0;
    CGColorRef textColor = [NSColor grayColor].CGColor;
    CGFloat fontSize = _font.pointSize;
    
    CGContextRef context = [NSGraphicsContext currentContext].CGContext;
    CGContextSaveGState(context);
    
    CGContextSetFont(context, cgFont);
    CGContextSetFontSize(context, fontSize);
    CGContextSetFillColorWithColor(context, textColor);
    
    CGGlyph digitGlyphs[10];
    const unichar numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    CTFontGetGlyphsForCharacters(ctFont, numbers, digitGlyphs, 10);
    
    CGSize advance;
    CTFontGetAdvancesForGlyphs(ctFont, kCTFontOrientationHorizontal, &digitGlyphs[8], &advance, 1);  // use '8' to get width
    CGFloat charWidth = advance.width;
    
    NSPoint relativePoint = [self convertPoint:NSZeroPoint fromView: textView];
    NSPoint inset = textView.textContainerOrigin;
    CGFloat ascent = CTFontGetAscent(ctFont);
    CGAffineTransform transform = CGAffineTransformIdentity;
    transform = CGAffineTransformScale(transform, 1.0, -1.0);
    transform = CGAffineTransformTranslate(transform, -padding, -relativePoint.y - inset.y - ascent);
    CGContextSetTextMatrix(context, transform);
    
    NSRect visibleRect = [[self scrollView] documentVisibleRect];
    visibleRect.size.height += fontSize;
    
    NSRange visibleGlyphRange = [layoutManager glyphRangeForBoundingRect:visibleRect inTextContainer:textView.textContainer];
    NSUInteger glyphCount = visibleGlyphRange.location;
    NSUInteger lineIndex = 0;
    NSUInteger lineNum = lineIndex + 1;
    NSUInteger lastLineNum = 0;
    
    __block NSUInteger count = 0;
    [string enumerateSubstringsInRange:NSMakeRange(0, [layoutManager characterIndexForGlyphAtIndex:visibleGlyphRange.location])
                               options:NSStringEnumerationByLines | NSStringEnumerationSubstringNotRequired
                            usingBlock:^(NSString * _Nullable substring, NSRange substringRange, NSRange enclosingRange, BOOL * _Nonnull stop) {
         count++;
    }];
    
    lineNum += count;
    
    for (NSUInteger glyphIndex = glyphCount; glyphIndex < NSMaxRange(visibleGlyphRange); ++lineIndex) {
        NSUInteger charIndex = [layoutManager characterIndexForGlyphAtIndex:glyphIndex];
        NSRange lineRange = [string lineRangeForRange:NSMakeRange(charIndex, 0)];
        NSRange lineCharacterRange = [layoutManager glyphRangeForCharacterRange:lineRange actualCharacterRange: nil];
        glyphIndex = NSMaxRange(lineCharacterRange);
        
        while (glyphCount < glyphIndex) {
            NSRange effectiveRange;
            NSRect lineRect = [layoutManager lineFragmentRectForGlyphAtIndex:glyphCount effectiveRange: &effectiveRange  withoutAdditionalLayout:YES];
            CGFloat y = -NSMinY(lineRect);
            
            if (lastLineNum != lineNum) {
                NSInteger digit = log10(lineNum) + 1.0;
                CGGlyph glyphs[digit];
                CGPoint positions[digit];
                for (int i = 0; i < digit; ++i) {
                    double fplace = (double)i;
                    int index = (lineNum % (NSUInteger)pow(10.0, fplace + 1.0)) / pow(10, fplace);
                    glyphs[i] = digitGlyphs[index];
                    positions[i] = CGPointMake(width - (i + 1) * charWidth, y);
                }
                CGContextShowGlyphsAtPositions(context, glyphs, positions, digit);
            }
            
            lastLineNum = lineNum;
            glyphCount = NSMaxRange(effectiveRange);
        }
        
        ++lineNum;
    }
        
    if (layoutManager.extraLineFragmentTextContainer) {
        NSRect lineRect = layoutManager.extraLineFragmentRect;
        CGFloat y = -NSMinY(lineRect);
        NSInteger digit = log10(lineNum) + 1.0;
        CGGlyph glyphs[digit];
        CGPoint positions[digit];
        for (int i = 0; i < digit; ++i) {
            double fplace = (double)i;
            int index = (lineNum % (NSUInteger)pow(10.0, fplace + 1.0)) / pow(10, fplace);
            glyphs[i] = digitGlyphs[index];
            positions[i] = CGPointMake(width - (i + 1) * charWidth, y);
        }
        CGContextShowGlyphsAtPositions(context, glyphs, positions, digit);
    }
    
    CGContextRestoreGState(context);
    
    NSUInteger length = MAX(log10(lineNum) + 1.0, 3);
    CGFloat requiredWidth = MAX(length * charWidth + 2.0 * padding, width);
    self.ruleThickness = requiredWidth;
}

@end

#pragma mark Refuse Drag and drop

@interface EditorTextView : NSTextView
@end

@implementation EditorTextView

- (NSArray<NSString *> *)acceptableDragTypes
{
    return nil;
}

@end
