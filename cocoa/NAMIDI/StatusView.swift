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
    
    var parseContext: ParseContextAdapter?
    var error: ParseError?
    var context: PlayerContext?
    
    func onParseFinished(namidi: NAMidiProxy, context: UnsafeMutablePointer<ParseContext>) {
        self.parseContext = ParseContextAdapter(contextRef: context)
        if self.parseContext!.hasError {
            error = self.parseContext!.error
        }
        else {
            error = nil
        }
        
        setNeedsDisplayInRect(self.errorInfomation!.bounds)
    }
    
    func onPlayerContextChanged(namidi: NAMidiProxy, context: UnsafeMutablePointer<PlayerContext>) {
        self.context = context.memory
        setNeedsDisplayInRect(self.playing!.bounds)
    }
    
    override func drawRect(dirtyRect: NSRect) {
        if nil != error {
            errorKind!.stringValue = error!.message.takeUnretainedValue()
            filename!.stringValue = (error!.filepath.takeUnretainedValue() as NSString).lastPathComponent
            location!.stringValue = String(format: "Line: %d   Column: %d", error!.location.firstLine, error!.location.firstColumn)
            errorInfomation!.hidden = false
        }
        else {
            errorInfomation!.hidden = true
        }
        
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