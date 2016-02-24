//
//  EventListViewController.m
//  NAMIDI
//
//  Created by abechan on 2/22/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EventListViewController.h"
#import "EventListRowView.h"
#import "Color.h"

@interface EventListViewController () <NSTableViewDataSource, NSTableViewDelegate> {
    NSMutableArray<MidiEventRepresentation *> *_events;
}

@property (weak) IBOutlet NSView *headerBackground;
@property (weak) IBOutlet NSView *headerLine;
@property (weak) IBOutlet NSTableView *tableView;
@end

@implementation EventListViewController

- (instancetype)init
{
    self = [super init];
    if (self) {
        _events = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.view.wantsLayer = YES;
    self.view.layer.backgroundColor = [Color darkGray].CGColor;
    
    _headerBackground.wantsLayer = YES;
    _headerBackground.layer.backgroundColor = [NSColor blackColor].CGColor;
    _headerLine.wantsLayer = YES;
    _headerLine.layer.backgroundColor = [Color gray].CGColor;
    
    _tableView.dataSource = self;
    _tableView.delegate = self;
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    [self buildEvents];
    [_trackSelection addObserver:self forKeyPath:@"selectionFlags" options:0 context:NULL];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [_trackSelection removeObserver:self forKeyPath:@"selectionFlags"];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context
{
    if (object == _trackSelection) {
        [self buildEvents];
    }
}

- (void)buildEvents
{
    [_events removeAllObjects];
    
    for (MidiEventRepresentation *event in _namidi.sequence.events) {
        if ([_trackSelection isTrackSelected:event.channel]) {
            [_events addObject:event];
        }
    }
    
    [_tableView reloadData];
}

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return _events.count;
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    MidiEventRepresentation *event = _events[row];
    MidiEvent *raw = event.raw;
    Location location = [_namidi.sequence locationByTick:raw->tick];
    
    EventListRowView *view = [tableView makeViewWithIdentifier:@"EventListRow" owner:nil];
    view.location = [NSString stringWithFormat:@"%03d:%02d:%03d", location.m, location.b, location.t];
    view.type = [NSString stringWithUTF8String:MidiEventType2String(raw->type)];
    
    if (MidiEventTypeNote == raw->type) {
        const char *labels[] = {
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        };
        
        NoteEvent *note = (NoteEvent *)raw;
        view.channel = [NSString stringWithFormat:@"%d", note->channel];
        view.note = [NSString stringWithFormat:@"%s%d", labels[note->noteNo % 12], note->noteNo / 12 - 2];
        view.gatetime = [NSString stringWithFormat:@"%d", note->gatetime];
        view.velocity = [NSString stringWithFormat:@"%d", note->velocity];
    }
    else {
        view.channel = nil;
        view.note = nil;
        view.gatetime = nil;
        view.velocity = nil;
        
        // TODO
        view.other = @"Copyright (c) 2016, Noriyoshi Abe. All right reserved.";
    }
    
    return view;
}

@end
