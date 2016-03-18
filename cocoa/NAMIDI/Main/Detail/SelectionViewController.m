//
//  SelectionViewController.m
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SelectionViewController.h"
#import "Color.h"

@interface SelectionViewController () <NAMidiRepresentationObserver> {
    NSArray<NSButton *> *_buttons;
}

@property (weak) IBOutlet NSButton *conductor;
@property (weak) IBOutlet NSButton *channel1;
@property (weak) IBOutlet NSButton *channel2;
@property (weak) IBOutlet NSButton *channel3;
@property (weak) IBOutlet NSButton *channel4;
@property (weak) IBOutlet NSButton *channel5;
@property (weak) IBOutlet NSButton *channel6;
@property (weak) IBOutlet NSButton *channel7;
@property (weak) IBOutlet NSButton *channel8;
@property (weak) IBOutlet NSButton *channel9;
@property (weak) IBOutlet NSButton *channel10;
@property (weak) IBOutlet NSButton *channel11;
@property (weak) IBOutlet NSButton *channel12;
@property (weak) IBOutlet NSButton *channel13;
@property (weak) IBOutlet NSButton *channel14;
@property (weak) IBOutlet NSButton *channel15;
@property (weak) IBOutlet NSButton *channel16;
@property (weak) IBOutlet NSButton *listOpener;
@end

@implementation SelectionViewController

- (void)awakeFromNib
{
    _buttons = @[
                 _conductor,
                 _channel1,
                 _channel2,
                 _channel3,
                 _channel4,
                 _channel5,
                 _channel6,
                 _channel7,
                 _channel8,
                 _channel9,
                 _channel10,
                 _channel11,
                 _channel12,
                 _channel13,
                 _channel14,
                 _channel15,
                 _channel16,
                 ];
    
    for (NSButton *button in _buttons) {
        button.action = @selector(buttonPressed:);
        button.target = self;
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.view.wantsLayer = YES;
    self.view.layer.backgroundColor = [Color darkGray].CGColor;
    ((NSButtonCell *)_listOpener.cell).highlightsBy = NSNoCellMask;
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self update];
    [_namidi addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [_namidi removeObserver:self];
}

- (void)update
{
    for (int i = 0; i < _buttons.count; ++i) {
        if ([_trackSelection isAvailable:i]) {
            _buttons[i].enabled = YES;
            _buttons[i].state = [_trackSelection isTrackSelected:i] ? NSOnState : NSOffState;
        }
        else {
            _buttons[i].enabled = NO;
            _buttons[i].state = NSOffState;
        }
    }
}

- (IBAction)buttonPressed:(NSButton *)sender
{
    int trackNo = (int)sender.tag;
    
    if (NSOnState == sender.state) {
        [_trackSelection select:trackNo];
    }
    else {
        [_trackSelection deselect:trackNo];
    }
}

- (BOOL)listOpened
{
    return NSOnState == _listOpener.state;
}

- (IBAction)toggleListOpened:(id)sender
{
    _listOpener.image = [NSImage imageNamed:NSOnState == _listOpener.state ? @"list_opened" : @"list_closed"];
    [_delegate selectionViewControllerDidToggleListOpened:self];
}

#pragma mark Menu Action

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (self.listOpened) {
        menuItem.title = NSLocalizedString(@"MainMenu_HideEventList", @"Hide Event List");
    }
    else {
        menuItem.title = NSLocalizedString(@"MainMenu_ShowEventList", @"Show Event List");
    }
    
    return YES;
}

- (IBAction)showEventList:(id)sender
{
    _listOpener.state = NSOnState == _listOpener.state ? NSOffState : NSOnState;
    [self toggleListOpened:self];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self update];
}

@end
