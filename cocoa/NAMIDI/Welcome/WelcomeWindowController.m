//
//  WelcomeWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeWindowController.h"
#import "ApplicationController.h"
#import "FileRepresentation.h"
#import "Color.h"

@interface WelcomeWindowController () <NSTableViewDataSource, NSWindowDelegate>
@property (weak) IBOutlet NSView *rightBackgroundView;
@property (weak) IBOutlet NSTableView *recentTableView;
@property (readonly) NSArray *recentDocuments;
@property (readonly) BOOL noRecentDocumentsHidden;
@property (weak) IBOutlet NSButton *gettingStartedButton;
@property (weak) IBOutlet NSButton *listenToExampleButton;
@property (weak) IBOutlet NSButton *createNewDocumentButton;
@property (weak) IBOutlet NSButton *welcomeSettingButton;
@property (weak) IBOutlet NSButton *openOtherDocumentButton;
@property (weak) IBOutlet NSView *separator1;
@property (weak) IBOutlet NSView *separator2;
@property (weak) IBOutlet NSView *separator3;
@property (weak) IBOutlet NSView *separator4;
@end

@implementation WelcomeWindowController

- (NSString *)windowNibName
{
    return @"WelcomeWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    self.window.titleVisibility = NSWindowTitleHidden;
    self.window.titlebarAppearsTransparent = YES;
    [self.window standardWindowButton:NSWindowCloseButton].superview.hidden = YES;
    
    self.window.movableByWindowBackground = YES;
    [self.window setFrameTopLeftPoint:CGPointZero];
    
    self.window.opaque = NO;
    self.window.backgroundColor = [NSColor clearColor];
    
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.backgroundColor = [Color darkGray].CGColor;
    self.window.contentView.layer.masksToBounds = YES;
    
    _rightBackgroundView.wantsLayer = YES;
    _rightBackgroundView.layer.backgroundColor = [Color ultraLightGray].CGColor;
    
    if (0 < self.countOfRecentDocuments) {
        [_recentTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
    }
    
    _gettingStartedButton.attributedTitle = [[NSAttributedString alloc] initWithString:_gettingStartedButton.title attributes:self.menuTextAttiributes];
    _listenToExampleButton.attributedTitle = [[NSAttributedString alloc] initWithString:_listenToExampleButton.title attributes:self.menuTextAttiributes];
    _createNewDocumentButton.attributedTitle = [[NSAttributedString alloc] initWithString:_createNewDocumentButton.title attributes:self.menuTextAttiributes];
    
    _welcomeSettingButton.attributedTitle = [[NSAttributedString alloc] initWithString:_welcomeSettingButton.title attributes:self.welcomeSettingTextAttiributes];
    _openOtherDocumentButton.attributedTitle = [[NSAttributedString alloc] initWithString:_openOtherDocumentButton.title attributes:self._openOtherDocumentButton];
    
    _separator1.backgroundColor = [Color gray];
    _separator2.backgroundColor = [Color gray];
    _separator3.backgroundColor = [Color gray];
    _separator4.backgroundColor = [Color gray];
}

- (NSDictionary *)menuTextAttiributes
{
    return @{NSForegroundColorAttributeName:[NSColor whiteColor],
              NSFontAttributeName:[NSFont systemFontOfSize:22.0 weight:-0.7],
              NSKernAttributeName:@(1.0)};
}

- (NSDictionary *)welcomeSettingTextAttiributes
{
    return @{NSForegroundColorAttributeName:[Color ultraLightGray],
             NSFontAttributeName:[NSFont systemFontOfSize:13.0 weight:-0.7]};
}

- (NSDictionary *)_openOtherDocumentButton
{
    return @{NSForegroundColorAttributeName:[Color darkGray],
             NSFontAttributeName:[NSFont systemFontOfSize:15.0 weight:-0.7]};
}

- (NSUInteger)countOfRecentDocuments
{
    return MIN(6, [NSDocumentController sharedDocumentController].recentDocumentURLs.count);
}

- (NSArray *)recentDocuments
{
    return [[[NSDocumentController sharedDocumentController].recentDocumentURLs mapObjectsUsingBlock:^id(id obj) {
        return [[FileRepresentation alloc] initWithURL:obj];
    }] subarrayWithRange:NSMakeRange(0, self.countOfRecentDocuments)];;
}

- (BOOL)noRecentDocumentsHidden
{
    return 0 < [self countOfRecentDocuments];
}

- (IBAction)closeButtonTapped:(id)sender
{
    [self close];
}

- (IBAction)gettingStartedButtonTapped:(id)sender
{
    [AppController showGettingStartedWindow];
}

- (IBAction)listenToExampleButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)createNewDocumentButtonTapped:(id)sender
{
    [AppController createDocument];
}

- (IBAction)helpButtonTapped:(id)sender
{
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"top" inBook:bookName];
}

- (IBAction)preferenceButtonTapped:(id)sender
{
    [AppController showPreferenceWindow];
}

- (IBAction)openOtherDocumentButtonTapped:(id)sender
{
    [AppController openDocument];
}

- (IBAction)recentTableViewSelectionChanged:(id)sender
{
    FileRepresentation *recentFile = self.recentDocuments[_recentTableView.selectedRow];
    [AppController openDocumentWithContentsOfURL:recentFile.url];
}

- (void)keyDown:(NSEvent *)theEvent
{
    unichar key = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];
    if (NSCarriageReturnCharacter == key && -1 != _recentTableView.selectedRow) {
        [self recentTableViewSelectionChanged:self];
    }
    else {
        [super keyDown:theEvent];
    }
}

@end