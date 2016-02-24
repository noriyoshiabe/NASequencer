//
//  Stub.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MidiSourceRepresentation : NSObject
@property (readonly, nonatomic) NSString *name;
@end

@interface PresetRepresentation : NSObject
@property (readonly, nonatomic) NSString *name;
@property (readonly, nonatomic) uint16_t bankNo;
@property (readonly, nonatomic) uint8_t programNo;
@end
