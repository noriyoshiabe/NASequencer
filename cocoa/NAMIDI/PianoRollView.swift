//
//  PianoRollView.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/21.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class PianoRollView : NSView, NAMidiObserverDelegate {
    
    let widthPerTick: CGFloat = 50.0 / 480.0
    let heightPerKey: CGFloat = 10.0
    
    let gridColorKey: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.1)
    let gridColorCKey: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.3)
    let gridColorBeat: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.1)
    let gridColorMeasure: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.3)

    var context: ParseContextSW?
    
    override var flipped: Bool {
        return true
    }
    
    override func drawRect(dirtyRect: NSRect) {
        drawGrid(dirtyRect)
        drawEvents(dirtyRect)
    }
    
    func drawGrid(dirtyRect: NSRect) {
        NSColor.whiteColor().setFill()
        NSRectFill(dirtyRect)
        
        NSBezierPath.setDefaultLineWidth(1)
        
        let left:CGFloat = CGRectGetMinX(dirtyRect)
        let right:CGFloat = CGRectGetMaxX(dirtyRect)
        
        for i in 0...127 {
            let y = heightPerKey * CGFloat(i + 1) + 0.5
            if dirtyRect.intersects(CGRectMake(left, y, right - left, y)) {
                (0 == (127 - i) % 12 ? gridColorCKey : gridColorKey).set()
                NSBezierPath.strokeLineFromPoint(CGPointMake(left, y), toPoint: CGPointMake(right, y))
            }
        }
        
        let top:CGFloat = CGRectGetMinY(dirtyRect)
        let bottom:CGFloat = CGRectGetMaxY(dirtyRect)
        
        var tick = Int32(left / widthPerTick)
        var tickTo =  Int32(right / widthPerTick)
        
        var location: Location = context!.sequence.location(tick)
        location.t = 0
        
        tick = context!.sequence.tick(location)
        var timeSign = context!.sequence.timeSign(tick)
        
        while tick <= tickTo {
            (1 == location.b ? gridColorMeasure : gridColorBeat).set()
            
            let x = round(CGFloat(tick + 120) * widthPerTick) + 0.5
            NSBezierPath.strokeLineFromPoint(CGPointMake(x, top), toPoint: CGPointMake(x, bottom))
            
            if timeSign.numerator < ++location.b {
                location.b = 1
                ++location.m
                timeSign = context!.sequence.timeSign(tick)
            }
            
            tick = context!.sequence.tick(location)
        }
    }
    
    func drawEvents(dirtyRect: NSRect) {
        for event:MidiEvent in self.context!.sequence.eventsSW {
            //let note:NoteEvent = reinterpretCast(event)
            println(event.tick)
        }
    }
    
    func onParseFinished(namidi: COpaquePointer, context: UnsafeMutablePointer<ParseContext>) {
        self.context = ParseContextSW(contextRef: context)
        
        let width:CGFloat = CGFloat(self.context!.sequence.length + 240) * widthPerTick
        self.frame = NSMakeRect(0, 0, CGFloat(width), (127 + 2) * heightPerKey)
        
        let parent:NSScrollView = superview?.superview? as NSScrollView
        setNeedsDisplayInRect(parent.convertRect(parent.bounds, toView: self))
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
