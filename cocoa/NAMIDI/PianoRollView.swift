//
//  PianoRollView.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/21.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class PianoRollView : NSView, NAMidiProxyDelegate {
    
    let widthPerTick: CGFloat = 50.0 / 480.0
    let heightPerKey: CGFloat = 10.0
    
    let gridColorKey: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.1)
    let gridColorCKey: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.3)
    let gridColorBeat: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.1)
    let gridColorMeasure: NSColor = NSColor(red: 0.2, green: 0.5, blue: 0.9, alpha: 0.3)
    
    let noteColors: [NSColor] = [
        NSColor(hue: 1.0 / 15.0 * 0,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 1,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 2,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 3,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 4,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 5,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 6,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 7,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 8,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 9,  saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 10, saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 11, saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 12, saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 13, saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 14, saturation: 1.0, brightness: 1.0, alpha: 0.5),
        NSColor(hue: 1.0 / 15.0 * 15, saturation: 1.0, brightness: 1.0, alpha: 0.5),
    ]
    
    let noteBorderColors: [NSColor] = [
        NSColor(hue: 1.0 / 15.0 * 0,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 1,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 2,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 3,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 4,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 5,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 6,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 7,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 8,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 9,  saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 10, saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 11, saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 12, saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 13, saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 14, saturation: 1.0, brightness: 1.0, alpha: 1.0),
        NSColor(hue: 1.0 / 15.0 * 15, saturation: 1.0, brightness: 1.0, alpha: 1.0),
    ]
    
    let currentPositionColor: NSColor = NSColor(red: 0.75, green: 0.75, blue: 0, alpha: 1)

    var context: ParseContextSW?
    var playerContext: PlayerContext?
    
    override var flipped: Bool {
        return true
    }
    
    override func drawRect(dirtyRect: NSRect) {
        let playing = CGFloat(round(CGFloat(self.playerContext!.tick + 120) * widthPerTick) + 0.5)
        
        if nil != self.context?.sequence {
            drawGrid(dirtyRect)
            drawEvents(dirtyRect, x: playing)
        }
        
        if nil != self.playerContext {
            drawPlayingPosition(dirtyRect, x: playing)
        }
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
        
        var location: Location = context!.sequence!.location(tick)
        location.t = 0
        
        tick = context!.sequence!.tick(location)
        var timeSign = context!.sequence!.timeSign(tick)
        
        while tick <= tickTo {
            (1 == location.b ? gridColorMeasure : gridColorBeat).set()
            
            let x = round(CGFloat(tick + 120) * widthPerTick) + 0.5
            NSBezierPath.strokeLineFromPoint(CGPointMake(x, top), toPoint: CGPointMake(x, bottom))
            
            if timeSign.numerator < ++location.b {
                location.b = 1
                ++location.m
                timeSign = context!.sequence!.timeSign(tick)
            }
            
            tick = context!.sequence!.tick(location)
        }
    }
    
    func drawEvents(dirtyRect: NSRect, x: CGFloat) {
        for event in self.context!.sequence!.eventsSW {
            switch event.memory.__.clazz.memory.typeID {
            case NoteEventClass.typeID:
                drawNote(dirtyRect, note: unsafeBitCast(event, UnsafePointer<NoteEvent>.self).memory, x: x)
            default:
                break
            }
        }
    }
    
    func drawNote(dirtyRect: NSRect, note: NoteEvent, x: CGFloat) {
        let left:CGFloat = round(CGFloat(note.__.tick + 120) * widthPerTick) + 0.5
        let right:CGFloat = round(CGFloat(note.__.tick + 120 + note.gatetime)) * widthPerTick + 0.5
        let y:CGFloat = CGFloat(127 - note.noteNo + 1) * heightPerKey + 0.5
        
        let rect:NSRect = NSMakeRect(left - 5, y - 5, right - left, 10)
        if CGRectInset(dirtyRect, 10, 0).intersects(rect) {
            let path:NSBezierPath = NSBezierPath(roundedRect: rect, xRadius: 5, yRadius: 5)
            if PLAYER_STATE_PLAYING.value == playerContext!.state.value
                    && CGRectIntersectsRect(rect, CGRectMake(x, 0, 0, self.bounds.size.height)) {
                noteBorderColors[Int(note.channel - 1)].set()
                path.fill()
            }
            else {
                noteColors[Int(note.channel - 1)].set()
                path.fill()
                noteBorderColors[Int(note.channel - 1)].set()
                path.stroke()
            }
        }
    }
    
    func drawPlayingPosition(dirtyRect: NSRect, x: CGFloat) {
        let top:CGFloat = CGRectGetMinY(dirtyRect)
        let bottom:CGFloat = CGRectGetMaxY(dirtyRect)
        currentPositionColor.set()
        NSBezierPath.strokeLineFromPoint(CGPointMake(x, top), toPoint: CGPointMake(x, bottom))
    }
    
    func onParseFinished(namidi: NAMidiProxy, context: UnsafeMutablePointer<ParseContext>) {
        let try: ParseContextSW = ParseContextSW(contextRef: context)
        if try.hasError {
            return
        }
        self.context = try
        
        dispatch_async(dispatch_get_main_queue()) {
            let width:CGFloat = CGFloat(self.context!.sequence!.length + 240) * self.widthPerTick
            self.frame = NSMakeRect(0, 0, CGFloat(width), (127 + 2) * self.heightPerKey)
            let parent:NSScrollView = self.superview?.superview? as NSScrollView
            self.setNeedsDisplayInRect(parent.convertRect(parent.bounds, toView: self))
        }
    }
    
    func onPlayerContextChanged(namidi: NAMidiProxy, context: UnsafeMutablePointer<PlayerContext>) {
        let parent:NSScrollView = superview?.superview? as NSScrollView
        self.playerContext = context.memory;
        dispatch_async(dispatch_get_main_queue()) {
            self.setNeedsDisplayInRect(parent.convertRect(parent.bounds, toView: self))
        }
    }
}
