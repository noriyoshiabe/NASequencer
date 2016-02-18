//
//  Stub.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MidiSourceRepresentation : NSObject
@property (readonly) NSString *name;
@end

@interface PresetRepresentation : NSObject
@property (readonly) NSString *name;
@property (readonly) uint16_t bankNo;
@property (readonly) uint8_t programNo;
@end
