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
    self.window.contentView.layer.cornerRadius = 10.0;
    self.window.contentView.layer.masksToBounds = YES;
    
    _rightBackgroundView.wantsLayer = YES;
    _rightBackgroundView.layer.backgroundColor = [Color ultraLightGray].CGColor;
    
    if (0 < self.countOfRecentDocuments) {
        [_recentTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
    }
}

- (NSUInteger)countOfRecentDocuments
{
    return [NSDocumentController sharedDocumentController].recentDocumentURLs.count;
}

- (NSArray *)recentDocuments
{
    return [[NSDocumentController sharedDocumentController].recentDocumentURLs mapObjectsUsingBlock:^id(id obj) {
        return [[FileRepresentation alloc] initWithURL:obj];
    }];
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