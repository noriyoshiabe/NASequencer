//
//  NAMidi-Bridge.h
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/19.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#ifndef NAMIDI_NAMidi_Bridge_h
#define NAMIDI_NAMidi_Bridge_h

#import <Foundation/Foundation.h>
#import "NAMidi.h"

typedef NS_ENUM(NSUInteger, EventType) {
    EventType_NoteEvent,
    EventType_Unknown,
};

extern EventType MidiEventGetType(MidiEvent *event);

#endif
