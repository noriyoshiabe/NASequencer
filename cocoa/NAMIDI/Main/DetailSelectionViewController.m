//
//  DetailSelectionViewController.m
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "DetailSelectionViewController.h"
#import "Color.h"

@interface DetailSelectionViewController () {
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
@end

@implementation DetailSelectionViewController

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
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    for (int i = 0; i < _buttons.count; ++i) {
        _buttons[i].state = [_trackSelection isTrackSelected:i] ? NSOnState : NSOffState;
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

@end
