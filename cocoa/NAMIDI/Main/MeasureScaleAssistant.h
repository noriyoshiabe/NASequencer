//
//  MeasureScaleAssistant.h
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MeasureScaleAssistant : NSObject
@property (assign, nonatomic) CGFloat scale;
- (CGFloat)measureOffset;
- (CGFloat)pixelPerTick;
- (CGFloat)tickPerPixel;
- (BOOL)scrollWheel:(NSEvent *)theEvent;
@end
