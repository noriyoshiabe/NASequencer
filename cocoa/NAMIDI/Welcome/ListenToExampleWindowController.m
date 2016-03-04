//
//  ListenToExampleWindowController.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ListenToExampleWindowController.h"
#import "ListenToExampleRowView.h"
#import "ApplicationController.h"

@interface ListenToExampleWindowController () <NSTableViewDataSource, NSTableViewDelegate>
@property (readonly, nonatomic) NSArray *examples;
@property (weak) IBOutlet NSTableView *tableView;
@end

@implementation ListenToExampleWindowController

- (NSString *)windowNibName
{
    return @"ListenToExampleWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    self.window.opaque = NO;
    self.window.backgroundColor = [NSColor clearColor];
    self.window.ignoresMouseEvents = NO;
    
    _tableView.wantsLayer = YES;
    _tableView.layer.cornerRadius = 5.0;
    _tableView.layer.masksToBounds = YES;
    
    _tableView.delegate = self;
    _tableView.dataSource = self;
    
    [_tableView reloadData];
}

- (NSArray *)examples
{
    return @[
             @{@"fileType": @"nmf", @"icon": [NSImage imageNamed:@"NMFIcon"], @"label": @"Note as MIDI"},
             @{@"fileType": @"abc", @"icon": [NSImage imageNamed:@"ABCIcon"], @"label": @"ABC Notation"},
             @{@"fileType": @"mml", @"icon": [NSImage imageNamed:@"MMLIcon"], @"label": @"Music Macro Language"},
             ];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [self close];
}

- (IBAction)tableViewSelectionChanged:(id)sender
{
    [AppController openExampleDocument:self.examples[_tableView.selectedRow][@"fileType"]];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.examples.count;
}

#pragma mark NSTableViewDelegate

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
    ListenToExampleRowView *view = [tableView makeViewWithIdentifier:@"ListenToExampleRow" owner:nil];
    view.info = self.examples[row];
    return view;
}

@end
