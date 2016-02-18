//
//  Stub.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Stub.h"

@implementation MidiSourceRepresentation

- (NSString *)name
{
    return [NSString stringWithFormat:@"Synth %p", self];
}

@end

@implementation PresetRepresentation

- (NSString *)name
{
    return [NSString stringWithFormat:@"Preset %p Bank:%d Prg:%d", self, self.bankNo, self.programNo];
}

- (uint16_t)bankNo
{
    return 120;
}

- (uint8_t)programNo
{
    return 2;
}

@end
