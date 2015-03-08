//
//  ParseContextAdapter.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/27.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation

class ParseContextAdapter : NSObject {
    private let contextRef: UnsafeMutablePointer<ParseContext>
    private let context: ParseContext
    private let sequenceAdapter: SequenceAdapter?
    
    init (contextRef: UnsafeMutablePointer<ParseContext>) {
        self.contextRef = contextRef
        self.context = contextRef.memory
        
        if UnsafeMutablePointer<Sequence>.null() != context.sequence {
            self.sequenceAdapter = SequenceAdapter(sequenceRef: context.sequence)
        }
        
        NARetain(UnsafeMutablePointer<Void>(contextRef))
    }
    
    deinit {
        NARelease(UnsafeMutablePointer<Void>(contextRef))
    }
    
    var sequence: SequenceAdapter {
        return sequenceAdapter!
    }
    
    var hasError: Bool {
        return nil != context.error
    }
    
    var error: ParseError {
        return context.error.memory
    }
}

class SequenceAdapter {
    private let sequenceRef: UnsafeMutablePointer<Sequence>
    private let sequence: Sequence
    
    let events: Events
    
    init (sequenceRef: UnsafeMutablePointer<Sequence>) {
        self.sequence = sequenceRef.memory
        self.events = Events(events: sequence.events)
        self.sequenceRef = sequenceRef
        NARetain(UnsafeMutablePointer<Void>(sequenceRef))
    }
    
    deinit {
        NARelease(UnsafeMutablePointer<Void>(sequenceRef))
    }
    
    func location(tick: Int32) -> Location {
        return TimeTableTick2Location(sequence.timeTable, tick)
    }
    
    func tick(location: Location) -> Int32 {
        return TimeTableLocation2Tick(sequence.timeTable, location.m, location.b, 0)
    }
    
    func timeSign(tick: Int32) -> (numerator:Int16, denominator:Int16) {
        var numerator: Int16 = 0
        var denominator: Int16 = 0
        TimeTableGetTimeSignByTick(sequence.timeTable, tick, &numerator, &denominator)
        return (numerator, denominator)
    }
    
    var length: Int32 {
        return sequence.length
    }
}

class EventsGenerator: GeneratorType {
    private let events: CFMutableArray
    private var index: Int
    private var count: Int
    
    init(events: CFMutableArray) {
        self.events = events
        count = CFArrayGetCount(self.events)
        index = 0
    }
    
    func next() -> UnsafeMutablePointer<MidiEvent>? {
        if count <= index {
            return nil
        }
        
        let ptr = CFArrayGetValueAtIndex(self.events, index++)
        return unsafeBitCast(ptr, UnsafeMutablePointer<MidiEvent>.self)
    }
}

class Events : SequenceType {
    let events: Unmanaged<CFMutableArray>!
    
    init (events: Unmanaged<CFMutableArray>) {
        self.events = events
    }
    
    func generate() -> EventsGenerator {
        return EventsGenerator(events: events.takeUnretainedValue())
    }
}