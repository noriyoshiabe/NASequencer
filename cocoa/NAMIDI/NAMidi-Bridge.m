//
//  NAMidi-Bridge.m
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/19.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#import "NAMidi-Bridge.h"

EventType MidiEventGetType(MidiEvent *event)
{
    const struct {
        const char *typeID;
        EventType eventType;
    } table[] = {
        {NoteEventID, EventType_NoteEvent},
    };
    
    const char *typeID = event->__.clazz->typeID;
    for (int i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        if (typeID == table[i].typeID) {
            return table[i].eventType;
        }
    }
    
    return EventType_Unknown;
}