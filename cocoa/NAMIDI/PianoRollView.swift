//
//  PianoRollView.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/21.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class PianoRollView : NSView, NAMidiObserverDelegate {
    
    override var flipped: Bool {
        return true
    }
    
    override func drawRect(dirtyRect: NSRect) {
        NSColor.whiteColor().setFill()
        NSRectFill(dirtyRect)
    }
    
    func onParseFinished(namidi: COpaquePointer, context: UnsafeMutablePointer<ParseContext>) {
        println(NACFDescription(context).takeUnretainedValue())
    }
    
    func onPlayerContextChanged(namidi: COpaquePointer, context: UnsafeMutablePointer<PlayerContext>) {
        let ctx:PlayerContext = context.memory;
        let min:Int = Int(ctx.usec / (1000 * 1000 * 60))
        let sec:Int = Int(ctx.usec / (1000 * 1000))
        let msec:Int = Int((ctx.usec / 1000) % 1000)
        
        switch ctx.state.value {
        case PLAYER_STATE_STOP.value:
            println("It's cool!!")
        case PLAYER_STATE_PLAYING.value:
            break
        default:
            break
        }
        
        println(String(format: "[%@] time: %02d:%02d:%03d  location: %03d:%02d:%03d  tempo=%.2f  %d/%d\r",
            PlayerState2CFString(Int32(ctx.state.value)).takeUnretainedValue() as String,
            min,
            sec,
            msec,
            ctx.location.m,
            ctx.location.b,
            ctx.location.t,
            ctx.tempo,
            ctx.numerator,
            ctx.denominator
            ))
    }
}
