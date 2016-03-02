//
//  SynthesizerRowView.m
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynthesizerRowView.h"

@interface SynthesizerRowView () {
    MidiSourceManagerRepresentation *_manager;
    MidiSourceDescriptionRepresentation *_description;
}
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
    
    _manager = [MidiSourceManagerRepresentation sharedInstance];
}

- (MidiSourceDescriptionRepresentation *)description
{
    return _description;
}

- (void)setDescription:(MidiSourceDescriptionRepresentation *)description
{
    _description = description;
    [self update];
}

- (void)update
{
    [self willChangeValueForKey:@"synthesizerName"];
    [self didChangeValueForKey:@"synthesizerName"];
    [self willChangeValueForKey:@"canUnload"];
    [self didChangeValueForKey:@"canUnload"];
    [self willChangeValueForKey:@"gain"];
    [self didChangeValueForKey:@"gain"];
    [self willChangeValueForKey:@"masterVolume"];
    [self didChangeValueForKey:@"masterVolume"];
}

- (NSString *)synthesizerName
{
    return _description.name;
}

- (BOOL)canUnload
{
    return ![_description.filepath isEqualToString:_manager.pathForDefaultMidiSource];
}

- (int)gain
{
    return _description.gain;
}

- (void)setGain:(int)gain
{
    [_manager setGainForDescription:_description gain:gain];
}

- (int)masterVolume
{
    return _description.masterVolume;
}

- (void)setMasterVolume:(int)masterVolume
{
    [_manager setMasterVolumeForDescription:_description masterVolume:masterVolume];
}

- (IBAction)unloadButtonPressed:(id)sender
{
    [_delegate synthesizerRowViewDidClickUnload:self];
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

