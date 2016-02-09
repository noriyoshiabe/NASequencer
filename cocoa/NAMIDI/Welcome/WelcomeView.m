//
//  WelcomeView.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeView.h"
#import "WelcomeViewRecentTableCell.h"

@interface WelcomeView () <NSTableViewDelegate, NSTableViewDataSource>
@property (weak) IBOutlet NSView *contentView;
@property (weak) IBOutlet NSTableView *recentTableView;
@end

@implementation WelcomeView

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    
    if (self) {
        if ([[NSBundle mainBundle] loadNibNamed:@"WelcomeView" owner:self topLevelObjects:nil]) {
            [self addSubview:self.contentView];
        }
    }
    
    return self;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    self.recentTableView.delegate = self;
    self.recentTableView.dataSource = self;
}

- (void)setRecentFiles:(NSArray *)recentFiles
{
    _recentFiles = recentFiles;
    [_recentTableView reloadData];
}

- (IBAction)closeButtonTapped:(id)sender
{
    [_delegate welcomeView:self closeButtonTapped:sender];
}

- (IBAction)gettingStartedButtonTapped:(id)sender
{
    [_delegate welcomeView:self gettingStartedButtonTapped:sender];
}

- (IBAction)listenToExampleButtonTapped:(id)sender
{
    [_delegate welcomeView:self listenToExampleButtonTapped:self];
}

- (IBAction)createNewDocumentButtonTapped:(id)sender
{
    [_delegate welcomeView:self createNewDocumentButtonTapped:sender];
}

- (IBAction)helpButtonTapped:(id)sender
{
    [_delegate welcomeView:self helpButtonTapped:sender];
}

- (IBAction)preferenceButtonTapped:(id)sender
{
    [_delegate welcomeView:self preferenceButtonTapped:sender];
}

- (IBAction)openOtherDocumentButtonTapped:(id)sender
{
    [_delegate welcomeView:self openOtherDocumentButtonTapped:sender];
}

- (IBAction)showWelcomeWhenStartsToggled:(NSButton *)sender
{
    [_delegate welcomeView:self showWelcomeWhenStartsToggled:sender];
}

- (IBAction)recentTableViewSelectionChanged:(id)sender
{
    [_delegate welcomeView:self recentTableViewSelectionChanged:(id)sender selectedRow:[_recentTableView selectedRow]];
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    WelcomeViewRecentTableCell *cellView = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    // TODO cellView.imageView
    cellView.textField.stringValue = [_recentFiles objectAtIndex:row];
    cellView.detailTextField.stringValue = [NSString stringWithFormat:@"%@ detail", [_recentFiles objectAtIndex:row] ];
    return cellView;
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.recentFiles.count;
}

@end
