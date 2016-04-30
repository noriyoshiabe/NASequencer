//
//  SynthesizerCellView.m
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynthesizerCellView.h"
#import "Preference.h"
#import "Color.h"

@interface SynthesizerCellView () {
    MidiSourceManagerRepresentation *_manager;
    MidiSourceDescriptionRepresentation *_description;
}
@property (strong) IBOutlet NSBox *contentBox;
@end

@implementation SynthesizerCellView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    if (!_contentBox) {
        [[NSBundle mainBundle] loadNibNamed:@"SynthesizerCellView" owner:self topLevelObjects:nil];
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
    [self willChangeValueForKey:@"available"];
    [self didChangeValueForKey:@"available"];
    [self willChangeValueForKey:@"canUnload"];
    [self didChangeValueForKey:@"canUnload"];
    [self willChangeValueForKey:@"gain"];
    [self didChangeValueForKey:@"gain"];
    [self willChangeValueForKey:@"masterVolume"];
    [self didChangeValueForKey:@"masterVolume"];
    [self willChangeValueForKey:@"errorMessage"];
    [self didChangeValueForKey:@"errorMessage"];
}

- (NSString *)synthesizerName
{
    if (_description.available) {
        return _description.name;
    }
    else if (_description.settings) {
        return _description.settings[kMidiSourceName];
    }
    else {
        return NSLocalizedString(@"Preference_UnknownSynthesizer", @"Unknown Synthesizer");
    }
}

- (BOOL)available
{
    return _description.available;
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
    [_manager saveMidiSourcePreference];
}

- (int)masterVolume
{
    return _description.masterVolume;
}

- (void)setMasterVolume:(int)masterVolume
{
    [_manager setMasterVolumeForDescription:_description masterVolume:masterVolume];
    [_manager saveMidiSourcePreference];
}

- (NSString *)errorMessage
{
    NSString *messageFormat = nil;
    
    switch (_description.error) {
        case MidiSourceDescriptionErrorNoError:
            break;
        case MidiSourceDescriptionErrorFileNotFound:
            messageFormat = NSLocalizedString(@"Preference_MidiSourceFileNotFound", @"\"%@\" does not exist.");
            break;
        case MidiSourceDescriptionErrorUnsupportedVersion:
            messageFormat = NSLocalizedString(@"Preference_MidiSourceFileUnsupportedVersion", @"Unsupported File Format Version \"%@\".");
            break;
        case MidiSourceDescriptionErrorInvalidFileFormat:
            messageFormat = NSLocalizedString(@"Preference_MidiSourceInvalidFileFormat", @"Invlalid File Format \"%@\".");
            break;
        case MidiSourceDescriptionErrorInvalidCRC:
            messageFormat = NSLocalizedString(@"Preference_MidiSourceInvalidCRC", @"Invlalid CRC \"%@\".");
            break;
    }
    
    if (messageFormat) {
        NSString *filepath = [_description.filepath stringByReplacingOccurrencesOfString:NSUserHomeDirectory() withString:@"~"];
        return [NSString stringWithFormat:messageFormat, filepath];
    }
    else {
        return nil;
    }
}

- (IBAction)unloadButtonPressed:(id)sender
{
    [_delegate synthesizerCellViewDidClickUnload:self];
}

- (IBAction)resetButtonPressed:(id)sender
{
    self.gain = -100;
    self.masterVolume = 0;
}

- (IBAction)deleteButtonPressed:(id)sender
{
    [_delegate synthesizerCellViewDidClickDelete:self];
}

- (IBAction)reloadButtonPressed:(id)sender
{
    [_delegate synthesizerCellViewDidClickReload:self];
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

#pragma mark For Error

@interface SynthesizerErrorBackgroundView : NSView
@end

@implementation SynthesizerErrorBackgroundView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    [[Color statusBackground] set];
    [[NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:4.0 yRadius:4.0] fill];
}

@end
