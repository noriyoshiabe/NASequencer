#pragma once

#import <Foundation/Foundation.h>

#import "NAMidi.h"

@interface Controller : NSObject<NAMidiObserver>

@property (nonatomic, strong) NAMidi *namidi;

- (void)run:(const char *)filepath repeat:(BOOL)repeat;
- (void)write:(const char *)filepath outfile:(const char *)outfile;

@end
