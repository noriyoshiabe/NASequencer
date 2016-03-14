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
#import "NoteTable.h"

@interface EventListViewController () <NSTableViewDataSource, NSTableViewDelegate, NAMidiRepresentationObserver> {
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

- (void)viewWillAppear
{
    [super viewWillAppear];
    [self buildEvents];
    [_trackSelection addObserver:self forKeyPath:@"selectionFlags" options:0 context:NULL];
    [_namidi addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [_trackSelection removeObserver:self forKeyPath:@"selectionFlags"];
    [_namidi removeObserver:self];
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
    view.channel = nil;
    view.note = nil;
    view.gatetime = nil;
    view.velocity = nil;
    view.other = nil;
    
    switch (raw->type) {
        case MidiEventTypeNote:
        {
            const char *labels[] = {
                "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
            };
            
            NoteEvent *note = (NoteEvent *)raw;
            view.channel = [NSString stringWithFormat:@"%d", note->channel];
            view.note = [NSString stringWithFormat:@"%s%d", labels[note->noteNo % 12], note->noteNo / 12 - 2];
            view.gatetime = [NSString stringWithFormat:@"%d", note->gatetime];
            view.velocity = [NSString stringWithFormat:@"%d", note->velocity];
        }
            break;
        case MidiEventTypeTempo:
        {
            TempoEvent *_event = (void *)raw;
            view.other = [NSString stringWithFormat:@"%.2f", _event->tempo];
        }
            break;
        case MidiEventTypeTime:
        {
            TimeEvent *_event = (void *)raw;
            view.other = [NSString stringWithFormat:@"%d/%d", _event->numerator, _event->denominator];
        }
            break;
        case MidiEventTypeKey:
        {
            KeyEvent *_event = (void *)raw;
            view.other = [NSString stringWithFormat:@"sf:%d mi:%d %s", _event->sf, _event->mi, MidiKeySign2String((MidiKeySign){_event->sf, _event->mi})];
        }
            break;
        case MidiEventTypeTitle:
        {
            TitleEvent *_event = (void *)raw;
            view.other = [NSString stringWithUTF8String:_event->text];
        }
            break;
        case MidiEventTypeCopyright:
        {
            CopyrightEvent *_event = (void *)raw;
            view.other = [NSString stringWithUTF8String:_event->text];
        }
            break;
        case MidiEventTypeMarker:
        {
            MarkerEvent *_event = (void *)raw;
            view.other = [NSString stringWithUTF8String:_event->text];
        }
            break;
        case MidiEventTypeBank:
        {
            BankEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Bank: %d", _event->bankNo];
        }
            break;
        case MidiEventTypeProgram:
        {
            ProgramEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Program: %d", _event->programNo];
        }
            break;
        case MidiEventTypeVolume:
        {
            VolumeEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Volume: %d", _event->value];
        }
            break;
        case MidiEventTypePan:
        {
            PanEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Pan: %d", _event->value];
        }
            break;
        case MidiEventTypeChorus:
        {
            ChorusEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Chorus: %d", _event->value];
        }
            break;
        case MidiEventTypeReverb:
        {
            ReverbEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Reverb: %d", _event->value];
        }
            break;
        case MidiEventTypeExpression:
        {
            ExpressionEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Expression: %d", _event->value];
        }
            break;
        case MidiEventTypeDetune:
        {
            DetuneEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Detune: %d", _event->value];
        }
            break;
        case MidiEventTypeSynth:
        {
            SynthEvent *_event = (void *)raw;
            view.channel = [NSString stringWithFormat:@"%d", _event->channel];
            view.other = [NSString stringWithFormat:@"Synth: %s", _event->identifier];
        }
            break;
    }
    
    return view;
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self buildEvents];
}

@end
