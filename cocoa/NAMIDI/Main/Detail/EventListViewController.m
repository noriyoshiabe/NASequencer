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
#import "ColorButton.h"
#import "NACString.h"

@interface EventListViewController () <NSTableViewDataSource, NSTableViewDelegate, NAMidiRepresentationObserver> {
    NSMutableArray<MidiEventRepresentation *> *_events;
}

@property (weak) IBOutlet NSView *headerBackground;
@property (weak) IBOutlet NSView *headerLine;
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet ColorButton *noteButton;
@property (weak) IBOutlet ColorButton *controlButton;
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
    self.view.layer.backgroundColor = [NSColor colorWithHexRGBA:0xAEB2BBFF].CGColor;
    
    _headerBackground.wantsLayer = YES;
    _headerBackground.layer.backgroundColor = [NSColor blackColor].CGColor;
    _headerLine.wantsLayer = YES;
    _headerLine.layer.backgroundColor = [Color pink].CGColor;
    
    _noteButton.activeBorderColor = [Color pink];
    _noteButton.activeTextColor = [NSColor whiteColor];
    _noteButton.activeBackgroundColor = [NSColor blackColor];
    _noteButton.inactiveBorderColor = [Color gray];
    _noteButton.inactiveTextColor = [Color gray];
    _noteButton.inactiveBackgroundColor = [Color darkGray];
    
    _controlButton.activeBorderColor = [Color pink];
    _controlButton.activeTextColor = [NSColor whiteColor];
    _controlButton.activeBackgroundColor = [NSColor blackColor];
    _controlButton.inactiveBorderColor = [Color gray];
    _controlButton.inactiveTextColor = [Color gray];
    _controlButton.inactiveBackgroundColor = [Color darkGray];
    
    _noteButton.state = NSOnState;
    _controlButton.state = NSOnState;
    
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
            if (MidiEventTypeNote == event.type) {
                if (NSOnState == _noteButton.state) {
                    [_events addObject:event];
                }
            }
            else {
                if (NSOnState == _controlButton.state) {
                    [_events addObject:event];
                }
            }
        }
    }
    
    [_tableView reloadData];
}

- (IBAction)filterButtonPressed:(id)sender
{
    [self buildEvents];
}

#pragma mark Menu Action

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (@selector(filterNoteEvent:) == menuItem.action) {
        if (NSOnState == _noteButton.state) {
            menuItem.title = NSLocalizedString(@"MainMenu_FilterNoteEvent", @"Filter Note Event");
        }
        else {
            menuItem.title = NSLocalizedString(@"MainMenu_UnfilterNoteEvent", @"Unfilter Note Event");
        }
    }
    else if (@selector(filterControlEvent:) == menuItem.action) {
        if (NSOnState == _controlButton.state) {
            menuItem.title = NSLocalizedString(@"MainMenu_FilterControlEvent", @"Filter Control Event");
        }
        else {
            menuItem.title = NSLocalizedString(@"MainMenu_UnfilterControlEvent", @"Unfilter Control Event");
        }
    }
    
    return YES;
}

- (IBAction)filterNoteEvent:(id)sender
{
    _noteButton.state = NSOnState == _noteButton.state ? NSOffState : NSOnState;
    [self buildEvents];
}

- (IBAction)filterControlEvent:(id)sender
{
    _controlButton.state = NSOnState == _controlButton.state ? NSOffState : NSOnState;
    [self buildEvents];
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
    Location _location = [_namidi.sequence locationByTick:raw->tick];
    
    EventListRowView *view = [tableView makeViewWithIdentifier:@"EventListRow" owner:nil];
    CGRect frame = view.frame;
    frame.size.width = tableView.bounds.size.width;
    view.frame = frame;
    
    char *location = NACStringFormat("%03d:%02d:%03d", _location.m, _location.b, _location.t);
    const char *type = NULL;
    char *channel;
    char *data;
    
    switch (raw->type) {
        case MidiEventTypeNote:
        {
            const char *labels[] = {
                "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
            };
            
            NoteEvent *note = (NoteEvent *)raw;
            type = "Note";
            channel = NACStringFormat("%d", note->channel);
            
            char *noteLabel = NACStringFormat("%s%d/%d", labels[note->noteNo % 12], note->noteNo / 12 - 2, note->noteNo);
            data = NACStringFormat("%-8s Vel:%3d Gt:%3d", noteLabel, note->velocity, note->gatetime);
            free(noteLabel);
        }
            break;
        case MidiEventTypeTempo:
        {
            TempoEvent *_event = (void *)raw;
            type = "Tempo Change";
            channel = NACStringFormat("  ");
            data = NACStringFormat("%.2f", _event->tempo);
        }
            break;
        case MidiEventTypeTime:
        {
            TimeEvent *_event = (void *)raw;
            type = "Time Signature";
            channel = NACStringFormat("  ");
            data = NACStringFormat("%d/%d", _event->numerator, _event->denominator);
        }
            break;
        case MidiEventTypeKey:
        {
            KeyEvent *_event = (void *)raw;
            type = "Key Signature";
            channel = NACStringFormat("");
            data = NACStringFormat("sf:%d mi:%d %s", _event->sf, _event->mi, MidiKeySign2String((MidiKeySign){_event->sf, _event->mi}));
        }
            break;
        case MidiEventTypeTitle:
        {
            TitleEvent *_event = (void *)raw;
            type = "Sequence Name";
            channel = NACStringFormat("  ");
            data = strdup(_event->text);
        }
            break;
        case MidiEventTypeCopyright:
        {
            CopyrightEvent *_event = (void *)raw;
            type = "Copyright Notice";
            channel = NACStringFormat("  ");
            data = strdup(_event->text);
        }
            break;
        case MidiEventTypeMarker:
        {
            MarkerEvent *_event = (void *)raw;
            type = "Marker";
            channel = NACStringFormat("  ");
            data = strdup(_event->text);
        }
            break;
        case MidiEventTypeBank:
        {
            BankEvent *_event = (void *)raw;
            type = "Bank Select";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->bankNo);
        }
            break;
        case MidiEventTypeProgram:
        {
            ProgramEvent *_event = (void *)raw;
            type = "Program Change";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->programNo);
        }
            break;
        case MidiEventTypeVolume:
        {
            VolumeEvent *_event = (void *)raw;
            type = "Volume";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypePan:
        {
            PanEvent *_event = (void *)raw;
            type = "Pan";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypeChorus:
        {
            ChorusEvent *_event = (void *)raw;
            type = "Chorus";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypeReverb:
        {
            ReverbEvent *_event = (void *)raw;
            type = "Reverb";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypeExpression:
        {
            ExpressionEvent *_event = (void *)raw;
            type = "Expression";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypePitch:
        {
            PitchEvent *_event = (void *)raw;
            type = "Pitch";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypeDetune:
        {
            DetuneEvent *_event = (void *)raw;
            type = "Detune";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypePitchSense:
        {
            PitchSenseEvent *_event = (void *)raw;
            type = "Pitch Sense";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%d", _event->value);
        }
            break;
        case MidiEventTypeSynth:
        {
            SynthEvent *_event = (void *)raw;
            type = "Synthesizer";
            channel = NACStringFormat("%d", _event->channel);
            data = NACStringFormat("%s", _event->identifier);
        }
            break;
    }
    
    view.content = [NSString stringWithFormat:@"%-16s  %-10s  %-2s       %s", type, location, channel, data];
    
    free(location);
    free(channel);
    free(data);
    
    return view;
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self buildEvents];
}

@end

#pragma mark Event list vertical line

@interface EventListScrollView : NSScrollView
@end

@implementation EventListScrollView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 0.5);
    CGContextSetStrokeColorWithColor(ctx, [Color  darkGray].CGColor);
    
    const CGFloat lineXs[] = {131.5, 218.5, 283.5};
    CGFloat height = self.bounds.size.height;
    
    for (int i = 0; i < sizeof(lineXs) / sizeof(lineXs[0]); ++i) {
        CGContextMoveToPoint(ctx, lineXs[i], 0);
        CGContextAddLineToPoint(ctx, lineXs[i], height);
        CGContextStrokePath(ctx);
    }
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

@end
