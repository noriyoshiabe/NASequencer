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

@class RecentTableView;
@protocol RecentTableViewDelegate <NSTableViewDelegate>
- (void)tableViewDidKeyDownEnter:(RecentTableView *)tableView;
@end

@interface RecentTableView : NSTableView
@end

@implementation RecentTableView

- (void)keyDown:(NSEvent *)theEvent
{
    unichar key = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];
    if (NSCarriageReturnCharacter == key && -1 != self.selectedRow) {
        [((id<RecentTableViewDelegate>)self.delegate) tableViewDidKeyDownEnter:self];
    }
    else {
        [super keyDown:theEvent];
    }
}

@end

@interface WelcomeViewRecentTableCell : NSTableCellView
@property (weak) IBOutlet NSTextField *detailTextField;
@end

@implementation WelcomeViewRecentTableCell
@end

@interface WelcomeWindowController () <RecentTableViewDelegate, NSTableViewDataSource>
@property (weak) IBOutlet RecentTableView *recentTableView;
@end

@implementation WelcomeWindowController

- (instancetype)init
{
    return [self initWithWindowNibName:@"WelcomeWindowController"];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.windowFrameAutosaveName = @"WelcomeWindowFrame";
    
    _recentTableView.delegate = self;
    _recentTableView.dataSource = self;
    
    [_recentTableView reloadData];
    
    if (0 < self.recentDocumentURLs.count) {
        [_recentTableView selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
    }
}

- (NSArray *)recentDocumentURLs
{
    return [NSDocumentController sharedDocumentController].recentDocumentURLs;
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
    [[ApplicationController sharedInstance] openDocumentWithContentsOfURL:self.recentDocumentURLs[[_recentTableView selectedRow]]];
}

#pragma mark RecentTableViewDelegate

- (void)tableViewDidKeyDownEnter:(RecentTableView *)tableView
{
    [[ApplicationController sharedInstance] openDocumentWithContentsOfURL:self.recentDocumentURLs[[_recentTableView selectedRow]]];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSURL *url = [self.recentDocumentURLs objectAtIndex:row];
    
    WelcomeViewRecentTableCell *cellView = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    cellView.imageView.image = [[NSWorkspace sharedWorkspace] iconForFileType:url.lastPathComponent.pathExtension];
    cellView.textField.stringValue = url.lastPathComponent;
    cellView.detailTextField.stringValue = [url.path.stringByDeletingLastPathComponent stringByReplacingOccurrencesOfString:[NSString stringWithCString:getpwuid(getuid())->pw_dir encoding:NSUTF8StringEncoding] withString:@"~"];
    return cellView;
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.recentDocumentURLs.count;
}

@end


#pragma mark InterfaceBuilder

@interface WelcomeWindow : NSWindow

@end

@implementation WelcomeWindow

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    self.titleVisibility = NSWindowTitleHidden;
    self.titlebarAppearsTransparent = YES;
    [self standardWindowButton:NSWindowCloseButton].superview.hidden = YES;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

- (BOOL)isMovableByWindowBackground
{
    return YES;
}

@end