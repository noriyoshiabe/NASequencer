//
//  EventListViewController.m
//  NAMIDI
//
//  Created by abechan on 2/22/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EventListViewController.h"
#import "Color.h"
#import "Stub.h"

@interface EventListCellView : NSTableCellView
@property (weak) IBOutlet NSTextField *location;
@property (weak) IBOutlet NSTextField *type;
@property (weak) IBOutlet NSTextField *channel;
@property (weak) IBOutlet NSTextField *note;
@property (weak) IBOutlet NSTextField *gatetime;
@property (weak) IBOutlet NSTextField *velocity;
@property (weak) IBOutlet NSTextField *other;
@end

@implementation EventListCellView
@end

@interface EventListViewController () <NSTableViewDataSource, NSTableViewDelegate> {
    NSMutableArray<MidiEventRepresentation *> *_events;
}

@property (weak) IBOutlet NSView *headerBackground;
@property (weak) IBOutlet NSView *headerLine;
@property (weak) IBOutlet NSTableView *tableView;
@property (strong, nonatomic) SequenceRepresentation *sequence;
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
    
    _sequence = [[SequenceRepresentation alloc] init];
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
    
    for (MidiEventRepresentation *event in _sequence.events) {
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
    Location location = [_sequence locationByTick:raw->tick];
    
    EventListCellView *view = [tableView makeViewWithIdentifier:@"EventRow" owner:self];
    view.location.stringValue = [NSString stringWithFormat:@"%03d:%02d:%03d", location.m, location.b, location.t];
    view.type.stringValue = [NSString stringWithUTF8String:MidiEventType2String(raw->type)];
    
    // TODO
    
    return view;
}

@end
