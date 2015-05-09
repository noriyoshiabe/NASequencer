//
//  StatusView.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class StatusView : NSView, NAMidiObserver {
    
    @IBOutlet var errorInfomation: NSView!
    @IBOutlet var errorKind: NSTextField!
    @IBOutlet var filename: NSTextField!
    @IBOutlet var location: NSTextField!

    @IBOutlet var playing: NSTextField!
    
    var namidi: NAMidi! {
        didSet {
            namidi.addObserver(self)
        }
    }
    
    var lastUsec: Int64 = 0
    
    func namidi(namidi: NAMidi!, onParseFinish sequence: Sequence!) {
        errorInfomation.hidden = true
    }
    
    func namidi(namidi: NAMidi!, onParseError filepath: String!, line: Int32, column: Int32, error: ParseError, info: UnsafePointer<Void>) {
        errorKind.stringValue = "TODO"
        filename.stringValue = filepath.lastPathComponent
        location.stringValue = "Line: \(line)   Column: \(column)"
        errorInfomation.hidden = false
    }
    
    func namidi(namidi: NAMidi!, player: Player!, notifyEvent playerEvent: PlayerEvent) {
        dispatch_async(dispatch_get_main_queue()) {
            self.needsDisplay = true
        }
    }
    
    override func drawRect(dirtyRect: NSRect) {
        let player = namidi.player
        let usec = player.usec
        let location = player.location
        
        let min:Int = Int(usec / (1000 * 1000 * 60))
        let sec:Int = Int(usec / (1000 * 1000))
        let msec:Int = Int((usec / 1000) % 1000)
        
        playing.stringValue = String(format: "%02d:%02d:%03d   %03d:%02d:%03d",
            min, sec, msec, location.m, location.b, location.t)
        
        if (player.playing) {
            setNeedsDisplayInRect(playing.frame)
        }
    }
}