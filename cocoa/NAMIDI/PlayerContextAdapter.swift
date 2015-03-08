//
//  PlayerContextAdapter.swift
//  NAMIDI
//
//  Created by abechan on 2015/03/08.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation

class PlayerContextAdapter : NSObject {
    private let contextRef: UnsafeMutablePointer<PlayerContext>
    private let context: PlayerContext
    
    let playing: CFArray
    
    init (contextRef: UnsafeMutablePointer<PlayerContext>, playing: CFArray) {
        self.contextRef = contextRef
        self.context = contextRef.memory
        self.playing = playing
        
        NARetain(UnsafeMutablePointer<Void>(contextRef))
    }
    
    deinit {
        NARelease(UnsafeMutablePointer<Void>(contextRef))
    }
    
    var tick: Int32 {
        return context.tick
    }
    
    var state: PlayerState {
        return context.state
    }
    
    var usec: Int64 {
        return context.usec
    }
    
    var location: Location {
        return context.location
    }
}