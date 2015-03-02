//
//  PianoRollView.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/21.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa
import QuartzCore

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

    var context: ParseContextAdapter?
    var playerContext: PlayerContext?
    
    var gridLayer: CGLayer?
    var notesLayer: CGLayer?
    var positionLayer: CGLayer?
    var playingLayer: CGLayer?
    
    override var flipped: Bool {
        return true
    }

    override func drawRect(dirtyRect: NSRect) {
        let contextPtr = NSGraphicsContext.currentContext()!.graphicsPort
        let context = unsafeBitCast(contextPtr, CGContext.self)
        CGContextClipToRect(context, dirtyRect)
        
        if nil != gridLayer {
            CGContextDrawLayerAtPoint(context, CGPointZero, gridLayer)
        }
        
        if nil != notesLayer {
            CGContextDrawLayerAtPoint(context, CGPointZero, notesLayer)
        }
        
        if nil != playerContext {
            drawPlayingLayer(context, dirtyRect: dirtyRect)
        }
        
        if nil != positionLayer {
            let tick = nil != self.playerContext ? self.playerContext!.tick : 0
            
            CGContextSaveGState(context)
            CGContextTranslateCTM(context, CGFloat(tick + 120) * widthPerTick, 0)
            CGContextDrawLayerAtPoint(context, CGPointZero, positionLayer)
            CGContextRestoreGState(context)
        }
    }
    
    func onParseFinished(namidi: NAMidiProxy, context: UnsafeMutablePointer<ParseContext>) {
        let try: ParseContextAdapter = ParseContextAdapter(contextRef: context)
        if try.hasError {
            return
        }
        self.context = try
        
        dispatch_async(dispatch_get_main_queue()) {
            let width:CGFloat = CGFloat(self.context!.sequence.length + 240) * self.widthPerTick
            self.frame = NSMakeRect(0, 0, CGFloat(width), (127 + 2) * self.heightPerKey)
            
            let contextPtr = NSGraphicsContext.currentContext()!.graphicsPort
            let context = unsafeBitCast(contextPtr, CGContext.self)
            
            self.gridLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
            self.notesLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
            self.positionLayer = CGLayerCreateWithContext(context, CGSizeMake(1, self.bounds.size.height), nil)
            self.playingLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
            
            self.drawGridLayer()
            self.drawNotesLayer()
            self.drawPositionLayer()
            
            let parent:NSScrollView = self.superview?.superview? as NSScrollView
            self.setNeedsDisplayInRect(parent.convertRect(parent.bounds, toView: self))
        }
    }
    
    func onPlayerContextChanged(namidi: NAMidiProxy, context: UnsafeMutablePointer<PlayerContext>) {
        self.playerContext = context.memory
        dispatch_async(dispatch_get_main_queue()) {
            let parent:NSScrollView = self.superview?.superview? as NSScrollView
            self.setNeedsDisplayInRect(parent.convertRect(parent.bounds, toView: self))
        }
    }
    
    func drawGridLayer() {
        let ctx:CGContext = CGLayerGetContext(gridLayer)
        
        CGContextSetLineWidth(ctx, 1.0)
        
        for i in 0...127 {
            let y = heightPerKey * CGFloat(i + 1)
            let color:CGColor = (0 == (127 - i) % 12 ? gridColorCKey : gridColorKey).CGColor
            CGContextSetStrokeColorWithColor(ctx, color)
            CGContextMoveToPoint(ctx, 0, y)
            CGContextAddLineToPoint(ctx, self.bounds.size.width, y)
            CGContextStrokePath(ctx)
        }
        
        var tick = Int32(0 / widthPerTick)
        var tickTo =  Int32(self.bounds.size.width / widthPerTick)
        
        var location: Location = context!.sequence.location(tick)
        location.t = 0
        
        tick = context!.sequence.tick(location)
        var timeSign = context!.sequence.timeSign(tick)
        
        while tick <= tickTo {
            let color:CGColor = (1 == location.b ? gridColorMeasure : gridColorBeat).CGColor
            let x = round(CGFloat(tick + 120) * widthPerTick)
            
            CGContextSetStrokeColorWithColor(ctx, color)
            CGContextMoveToPoint(ctx, x, 0)
            CGContextAddLineToPoint(ctx, x, self.bounds.size.height)
            CGContextStrokePath(ctx)
            
            if timeSign.numerator < ++location.b {
                location.b = 1
                ++location.m
                timeSign = context!.sequence.timeSign(tick)
            }
            
            tick = context!.sequence.tick(location)
        }
    }
    
    func drawNotesLayer() {
        let ctx:CGContext = CGLayerGetContext(notesLayer)
        
        for event in self.context!.sequence.events {
            switch event.memory.__.clazz.memory.typeID {
            case NoteEventClass.typeID:
                drawNote(ctx, note: unsafeBitCast(event, UnsafePointer<NoteEvent>.self).memory, active: false)
            default:
                break
            }
        }
    }
    
    func drawNote(ctx: CGContext, note: NoteEvent, active: Bool) {
        let left:CGFloat = round(CGFloat(note.__.tick + 120) * widthPerTick)
        let right:CGFloat = round(CGFloat(note.__.tick + 120 + note.gatetime)) * widthPerTick
        let y:CGFloat = CGFloat(127 - note.noteNo + 1) * heightPerKey
        
        let rect:NSRect = NSMakeRect(left - 5, y - 5, right - left, 10)
        let lx = CGRectGetMinX(rect)
        let cx = CGRectGetMidX(rect)
        let rx = CGRectGetMaxX(rect)
        let by = CGRectGetMinY(rect)
        let cy = CGRectGetMidY(rect)
        let ty = CGRectGetMaxY(rect)
        let radius:CGFloat = 5.0
        
        CGContextSetLineWidth(ctx, 1.0)
        
        if active {
            CGContextMoveToPoint(ctx, lx, cy)
            CGContextAddArcToPoint(ctx, lx, by, cx, by, radius)
            CGContextAddArcToPoint(ctx, rx, by, rx, cy, radius)
            CGContextAddArcToPoint(ctx, rx, ty, cx, ty, radius)
            CGContextAddArcToPoint(ctx, lx, ty, lx, cy, radius)
            CGContextClosePath(ctx)
            
            CGContextSetFillColorWithColor(ctx, noteBorderColors[Int(note.channel - 1)].CGColor)
            CGContextFillPath(ctx)
        }
        else {
            CGContextMoveToPoint(ctx, lx, cy)
            CGContextAddArcToPoint(ctx, lx, by, cx, by, radius)
            CGContextAddArcToPoint(ctx, rx, by, rx, cy, radius)
            CGContextAddArcToPoint(ctx, rx, ty, cx, ty, radius)
            CGContextAddArcToPoint(ctx, lx, ty, lx, cy, radius)
            CGContextClosePath(ctx)
            
            CGContextSetFillColorWithColor(ctx, noteColors[Int(note.channel - 1)].CGColor)
            CGContextFillPath(ctx)
            
            CGContextMoveToPoint(ctx, lx, cy)
            CGContextAddArcToPoint(ctx, lx, by, cx, by, radius)
            CGContextAddArcToPoint(ctx, rx, by, rx, cy, radius)
            CGContextAddArcToPoint(ctx, rx, ty, cx, ty, radius)
            CGContextAddArcToPoint(ctx, lx, ty, lx, cy, radius)
            CGContextClosePath(ctx)
            
            CGContextSetStrokeColorWithColor(ctx, noteBorderColors[Int(note.channel - 1)].CGColor)
            CGContextStrokePath(ctx)
        }
    }
    
    func drawPositionLayer() {
        let ctx:CGContext = CGLayerGetContext(positionLayer)
        
        CGContextSetLineWidth(ctx, 1.0)
        
        CGContextSetStrokeColorWithColor(ctx, currentPositionColor.CGColor)
        CGContextMoveToPoint(ctx, 0, 0)
        CGContextAddLineToPoint(ctx, 0, self.bounds.size.height)
        CGContextStrokePath(ctx)
    }
    
    func drawPlayingLayer(ctx: CGContext, dirtyRect: NSRect) {
        if PLAYER_STATE_PLAYING.value != playerContext!.state.value {
            return
        }
        
        let playing = CGFloat(round(CGFloat(self.playerContext!.tick + 120) * widthPerTick) + 0.5)
        
        var arr:CFMutableArray = playerContext!.playing.takeUnretainedValue()
        var size = CFArrayGetCount(arr)
        for var i = 0; i < size; ++i {
            var ptr = CFArrayGetValueAtIndex(arr, i)
            var note:NoteEvent = unsafeBitCast(ptr, UnsafeMutablePointer<NoteEvent>.self).memory
            if self.playerContext!.tick > note.__.tick + note.gatetime {
                continue
            }
            else if self.playerContext!.tick < note.__.tick {
                break
            }
            
            let left:CGFloat = round(CGFloat(note.__.tick + 120) * widthPerTick) + 0.5
            let right:CGFloat = round(CGFloat(note.__.tick + 120 + note.gatetime)) * widthPerTick + 0.5
            let y:CGFloat = CGFloat(127 - note.noteNo + 1) * heightPerKey + 0.5
            let rect:NSRect = NSMakeRect(left - 5, y - 5, right - left, 10)
            if CGRectIntersectsRect(rect, CGRectMake(playing, 0, 0, self.bounds.size.height)) {
                drawNote(ctx, note: note, active: true)
            }
        }
    }
}
