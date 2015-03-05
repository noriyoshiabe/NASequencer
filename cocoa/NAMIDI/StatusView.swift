//
//  StatusView.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class StatusView : NSView, NAMidiProxyDelegate {
    
    @IBOutlet var errorInfomation: NSView?
    @IBOutlet var errorKind: NSTextField?
    @IBOutlet var filename: NSTextField?
    @IBOutlet var location: NSTextField?

    @IBOutlet var playing: NSTextField?
    
    var lastUsec: Int64 = 0
    
    func onParseFinished(namidi: NAMidiProxy, context: UnsafeMutablePointer<ParseContext>) {
        let parseContext = ParseContextAdapter(contextRef: context)
        if parseContext.hasError {
            let error = parseContext.error
            errorKind!.stringValue = error.message.takeUnretainedValue()
            filename!.stringValue = (error.filepath.takeUnretainedValue() as NSString).lastPathComponent
            location!.stringValue = String(format: "Line: %d   Column: %d", error.location.firstLine, error.location.firstColumn)
            errorInfomation!.hidden = false
        }
        else {
            errorInfomation!.hidden = true
        }
        
    }
    
    func onPlayerContextChanged(namidi: NAMidiProxy, context: UnsafeMutablePointer<PlayerContext>, playingNotes:CFArray!) {
        let context = context.memory
        
        if 10000 > abs(context.usec - lastUsec) {
            return
        }
        
        let min:Int = Int(context.usec / (1000 * 1000 * 60))
        let sec:Int = Int(context.usec / (1000 * 1000))
        let msec:Int = Int((context.usec / 1000) % 1000)
        
        playing!.stringValue = String(format: "%02d:%02d:%03d   %03d:%02d:%03d",
            min,
            sec,
            msec,
            context.location.m,
            context.location.b,
            context.location.t
        )
        
        lastUsec = context.usec
    }
}