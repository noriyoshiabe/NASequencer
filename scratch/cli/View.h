#pragma once

#import <Foundation/Foundation.h>

#import "NAMidi.h"

@interface View : NSObject<NAMidiObserver>
@property (nonatomic, strong) NAMidi *namidi;
@end
