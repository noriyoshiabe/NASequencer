//
//  MeasureScaleAssistant.h
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NAMidiRepresentation.h"

@interface MeasureScaleAssistant : NSObject
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (assign, nonatomic) CGFloat scale;
- (CGFloat)measureOffset;
- (CGFloat)pixelPerTick;
- (CGFloat)tickPerPixel;
- (BOOL)scrollWheel:(NSEvent *)theEvent;
- (CGFloat)viewWidth;
@end
