//
//  WelcomeWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeWindowController.h"
#import "ApplicationController.h"

#include <pwd.h>

@interface RecentFile : NSObject
@property (readonly) NSURL *url;
@property (readonly) NSImage *fileTypeIcon;
@property (readonly) NSString *filename;
@property (readonly) NSString *directory;
@end

@implementation RecentFile

- (instancetype)initWithURL:(NSURL *)url
{
    self = [super init];
    if (self) {
        _url = url;
    }
    return self;
}

- (NSImage *)fileTypeIcon
{
    return [[NSWorkspace sharedWorkspace] iconForFileType:_url.lastPathComponent.pathExtension];
}

- (NSString *)filename
{
    return _url.lastPathComponent;
}

- (NSString *)directory
{
    return [_url.path.stringByDeletingLastPathComponent stringByReplacingOccurrencesOfString:[NSString stringWithCString:getpwuid(getuid())->pw_dir encoding:NSUTF8StringEncoding] withString:@"~"];
}

@end

@interface WelcomeWindowController () <NSTableViewDataSource>
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
    self.windowFrameAutosaveName = @"WelcomeWindowFrame";
    
    self.window.titleVisibility = NSWindowTitleHidden;
    self.window.titlebarAppearsTransparent = YES;
    self.window.movableByWindowBackground = YES;
    [self.window standardWindowButton:NSWindowCloseButton].superview.hidden = YES;
    
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
        return [[RecentFile alloc] initWithURL:obj];
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
    [[ApplicationController sharedInstance] showGettingStartedWindow];
}

- (IBAction)listenToExampleButtonTapped:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)createNewDocumentButtonTapped:(id)sender
{
    [[ApplicationController sharedInstance] createDocument];
}

- (IBAction)helpButtonTapped:(id)sender
{
    NSString *bookName = [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleHelpBookName"];
    [[NSHelpManager sharedHelpManager] openHelpAnchor:@"top" inBook:bookName];
}

- (IBAction)preferenceButtonTapped:(id)sender
{
    [[ApplicationController sharedInstance] showPreferenceWindow];
}

- (IBAction)openOtherDocumentButtonTapped:(id)sender
{
    [[ApplicationController sharedInstance] openDocument];
}

- (IBAction)showWelcomeWhenStartsToggled:(NSButton *)sender
{
    NSLog(@"%s %s", NSOnState == sender.state ? "ON" : "OFF", __func__);
}

- (IBAction)recentTableViewSelectionChanged:(id)sender
{
    RecentFile *recentFile = self.recentDocuments[_recentTableView.selectedRow];
    [[ApplicationController sharedInstance] openDocumentWithContentsOfURL:recentFile.url];
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