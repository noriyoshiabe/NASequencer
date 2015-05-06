#pragma once

#import <Foundation/Foundation.h>

#import "NAMidi.h"

@interface Controller : NSObject<NAMidiObserver>
@property (nonatomic, strong) NAMidi *namidi;
- (void)run:(const char *)filepath;
@end
