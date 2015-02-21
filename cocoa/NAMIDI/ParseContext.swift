//
//  ParseContext.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation

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
}