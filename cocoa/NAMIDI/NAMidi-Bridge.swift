//
//  NAMidi-Bridge.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation

class NAMidiSW {
    private let namidi: COpaquePointer
    private var observers: [NAMidiObserver] = []
    
    init() {
        namidi = NAMidiCreate()
    }
    
    deinit {
        NARelease(UnsafeMutablePointer<Void>(namidi))
    }
    
    func addObserverDelegate(observerDelegate: NAMidiObserverDelegate) {
        let observer = NAMidiObserver(delegate: observerDelegate)
        NAMidiAddObserver(namidi, observer.observerBridge)
        observers.append(observer)
    }
    
    func setFile(url: NSURL) {
        NAMidiSetFile(namidi, url.path! as CFString)
    }
    
    func parse() {
        NAMidiParse(namidi)
    }
    
    func play() {
        NAMidiPlay(namidi)
    }
    
    func stop() {
        NAMidiStop(namidi)
    }
    
    func playPause() {
        NAMidiPlayPause(namidi)
    }
    
    func rewind() {
        NAMidiRewind(namidi)
    }
    
    func forward() {
        NAMidiForward(namidi)
    }
    
    func backward() {
        NAMidiBackward(namidi)
    }
}

class ParseContextSW {
    let context: ParseContext
    let contextRef: UnsafeMutablePointer<ParseContext>
    
    init (contextRef: UnsafeMutablePointer<ParseContext>) {
        self.context = contextRef.memory
        self.contextRef = contextRef
        NARetain(UnsafeMutablePointer<Void>(contextRef))
    }
    
    var sequence: Sequence {
        return context.sequence.memory
    }
    
    deinit {
        NARelease(UnsafeMutablePointer<Void>(contextRef))
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
    
    func next() -> MidiEvent? {
        if count <= index {
            return nil
        }
        
        let ptr = CFArrayGetValueAtIndex(self.events, index++)
        return unsafeBitCast(ptr, UnsafeMutablePointer<MidiEvent>.self).memory
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

extension Sequence {
    func location(tick: Int32) -> Location {
        return TimeTableTick2Location(timeTable, tick)
    }
    
    func tick(location: Location) -> Int32 {
        return TimeTableLocation2Tick(timeTable, location.m, location.b, 0)
    }
    
    func timeSign(tick: Int32) -> (numerator:Int16, denominator:Int16) {
        var numerator: Int16 = 0
        var denominator: Int16 = 0
        TimeTableGetTimeSignByTick(timeTable, tick, &numerator, &denominator)
        return (numerator, denominator)
    }
    
    var eventsSW: Events {
        get {
            return Events(events: self.events)
        }
    }
}