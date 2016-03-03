//
//  SynthesizerCellView.h
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MidiSourceManagerRepresentation.h"

@class SynthesizerCellView;
@protocol SynthesizerCellViewDelegate <NSObject>
- (void)synthesizerCellViewDidClickUnload:(SynthesizerCellView *)view;
- (void)synthesizerCellViewDidClickDelete:(SynthesizerCellView *)view;
- (void)synthesizerCellViewDidClickReload:(SynthesizerCellView *)view;
@end

@interface SynthesizerCellView : NSTableCellView
@property (weak, nonatomic) id<SynthesizerCellViewDelegate> delegate;
@property (strong, nonatomic) MidiSourceDescriptionRepresentation *description;
@property (readonly, nonatomic) NSString *synthesizerName;
@property (readonly, nonatomic) BOOL canUnload;
@property (readonly, nonatomic) BOOL available;
@property (assign, nonatomic) int gain;
@property (assign, nonatomic) int masterVolume;
@property (readonly, nonatomic) NSString *errorMessage;
@end
