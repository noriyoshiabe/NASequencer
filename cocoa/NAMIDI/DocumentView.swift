//
//  DocumentView.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

protocol DocumentViewDelegate : NSObjectProtocol {
    func draggingEntered(sender: DocumentView, draggingInfo: NSDraggingInfo) -> NSDragOperation
    func performDragOperation(sender: DocumentView, draggingInfo: NSDraggingInfo) -> Bool
}

class DocumentView : NSView, NAMidiObserverDelegate {

    override func awakeFromNib() {
        super.awakeFromNib()
        registerForDraggedTypes([NSFilenamesPboardType])
    }
    
    override func draggingEntered(sender: NSDraggingInfo) -> NSDragOperation {
        return AppDelegate.sharedApplication().draggingEntered(self, draggingInfo: sender)
    }
    
    override func performDragOperation(sender: NSDraggingInfo) -> Bool {
        return AppDelegate.sharedApplication().performDragOperation(self, draggingInfo: sender)
    }
    
    func onParseFinished(namidi: COpaquePointer, context: UnsafeMutablePointer<ParseContext>) {
        println(NACFDescription(context).takeUnretainedValue())
    }
    
    func onPlayerContextChanged(namidi: COpaquePointer, context: UnsafeMutablePointer<PlayerContext>) {
        let ctx:PlayerContext = context.memory;
        let min:Int = Int(ctx.usec / (1000 * 1000 * 60))
        let sec:Int = Int(ctx.usec / (1000 * 1000))
        let msec:Int = Int((ctx.usec / 1000) % 1000)
        
        println(String(format: "[%@] time: %02d:%02d:%03d  location: %03d:%02d:%03d  tempo=%.2f  %d/%d\r",
            PlayerState2CFString(ctx.state).takeUnretainedValue() as String,
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
