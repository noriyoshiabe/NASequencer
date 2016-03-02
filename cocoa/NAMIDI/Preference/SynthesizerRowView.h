//
//  SynthesizerRowView.h
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SynthesizerRowView : NSTableRowView
@property (readonly, nonatomic) NSString *synthesizerName;
@property (readonly, nonatomic) BOOL canUnload;
@property (assign, nonatomic) int gain;
@property (assign, nonatomic) int masterVolume;
@end
