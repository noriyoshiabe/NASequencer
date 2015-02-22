//
//  StatusView.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class StatusView : NSView, NAMidiObserverDelegate {
    
    @IBOutlet var playing: NSTextField?
    var context: PlayerContext?
    
    func onParseFinished(namidi: COpaquePointer, context: UnsafeMutablePointer<ParseContext>) {
    }
    
    func onPlayerContextChanged(namidi: COpaquePointer, context: UnsafeMutablePointer<PlayerContext>) {
        self.context = context.memory
        setNeedsDisplayInRect(playing!.bounds)
    }
    
    override func drawRect(dirtyRect: NSRect) {
        if nil == context {
            return
        }
        
        let min:Int = Int(context!.usec / (1000 * 1000 * 60))
        let sec:Int = Int(context!.usec / (1000 * 1000))
        let msec:Int = Int((context!.usec / 1000) % 1000)
        
        playing!.stringValue = String(format: "%02d:%02d:%03d   %03d:%02d:%03d",
            min,
            sec,
            msec,
            context!.location.m,
            context!.location.b,
            context!.location.t
        )
    }
}