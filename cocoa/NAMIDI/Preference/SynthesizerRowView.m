//
//  SynthesizerRowView.m
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynthesizerRowView.h"

@interface SynthesizerRowView ()
@property (strong) IBOutlet NSBox *contentBox;
@end

@implementation SynthesizerRowView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    if (!_contentBox) {
        [[NSBundle mainBundle] loadNibNamed:@"SynthesizerRowView" owner:self topLevelObjects:nil];
        [self addSubview:_contentBox];
    }
}

- (NSString *)synthesizerName
{
    return @"TEST GeneralUser GS Live/Audigy version 1.44";
}

- (void)setGain:(int)gain
{
    _gain = gain;
    __Trace__
}

- (void)setMasterVolume:(int)masterVolume
{
    _masterVolume = masterVolume;
    __Trace__
}

- (IBAction)unloadButtonPressed:(id)sender
{
    __Trace__
}

- (IBAction)resetButtonPressed:(id)sender
{
    self.gain = -100;
    self.masterVolume = 0;
}

@end

#pragma mark For keyboard controll

@interface SynthesizerSlider : NSSlider
@end

@implementation SynthesizerSlider

- (double)altIncrementValue
{
    return 1.0;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    [super mouseDown:theEvent];
    [self.window makeFirstResponder:self];
}

@end

