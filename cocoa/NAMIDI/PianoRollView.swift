//
//  PianoRollView.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/21.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa
import QuartzCore

class PianoRollView : NSView, NAMidiObserver {
    
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

    var namidi: NAMidi! {
        didSet {
            namidi.addObserver(self)
        }
    }
    
    var gridLayer: CGLayer!
    var noteLayer: CGLayer!
    
    var playingLine: CALayer!
    
    var scrolling: Bool = false
    
    override var flipped: Bool {
        return true
    }

    override func awakeFromNib() {
        playingLine = CALayer()
        playingLine.backgroundColor = currentPositionColor.CGColor;
        playingLine.frame = NSMakeRect(0, 0, 1, round((127 + 2) * self.heightPerKey))
        playingLine.hidden = true
        self.layer!.addSublayer(playingLine)
    }
    
    override func drawRect(dirtyRect: NSRect) {
        let contextPtr = NSGraphicsContext.currentContext()!.graphicsPort
        let context = unsafeBitCast(contextPtr, CGContext.self)
        
        CGContextClipToRect(context, dirtyRect)
        
        if nil != gridLayer {
            CGContextDrawLayerAtPoint(context, CGPointZero, gridLayer)
        }
        
        if nil != noteLayer {
            CGContextDrawLayerAtPoint(context, CGPointZero, noteLayer)
        }
    }
    
    func namidi(namidi: NAMidi!, onParseFinish sequence: Sequence!) {
        dispatch_async(dispatch_get_main_queue()) {
            let width = CGFloat(namidi.sequence.length + 240) * self.widthPerTick
            self.frame = NSMakeRect(0, 0, round(width), round((127 + 2) * self.heightPerKey))
            
            let context = NSGraphicsContext.currentContext()?.CGContext
            
            self.gridLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
            self.drawGridLayer()
            self.noteLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
            self.drawNoteLayer()
            
            self.updatePlayingPosition()
            self.playingLine.hidden = false
            
            self.needsDisplay = true
        }
    }
    
    func currentX() -> CGFloat {
        return round(CGFloat(namidi.player.tick + 120) * widthPerTick)
    }
    
    func namidi(namidi: NAMidi!, player: Player!, notifyEvent playerEvent: PlayerEvent) {
        dispatch_async(dispatch_get_main_queue()) {
            self.updatePlayingPosition()
            self.autoScroll()
        }
    }
    
    func namidi(namidi: NAMidi!, player: Player!, didSendNoteOn noteEvent: NoteEvent!) {
        dispatch_async(dispatch_get_main_queue()) {
            let ctx = CGLayerGetContext(self.noteLayer)
            self.drawNote(ctx, note:noteEvent, active: true)
        }
    }
    
    func namidi(namidi: NAMidi!, player: Player!, didSendNoteOff noteEvent: NoteEvent!) {
        dispatch_async(dispatch_get_main_queue()) {
            let ctx = CGLayerGetContext(self.noteLayer)
            self.drawNote(ctx, note:noteEvent, active: false)
        }
    }
    
    func namidi(namidi: NAMidi!, player: Player!, onClock tick: UInt32, usec: Int64, location: Location) {
        dispatch_async(dispatch_get_main_queue()) {
            self.updatePlayingPosition()
            
            if namidi.player.playing {
                self.autoScroll()
            }
        }
    }
    
    func autoScroll() {
        if scrolling {
            return
        }
        
        let toX = currentX()
        
        let parent: NSScrollView = self.superview?.superview as! NSScrollView
        if !parent.documentVisibleRect.intersects(CGRectMake(toX, 0, 0, self.bounds.size.height)) {
            scrolling = true
            
            NSAnimationContext.runAnimationGroup(
                { (context:NSAnimationContext!) -> Void in
                    context.duration = 0.5
                    parent.contentView.animator().setBoundsOrigin(CGPointMake(toX - round(120 * self.widthPerTick), parent.documentVisibleRect.origin.y))
                },
                completionHandler: {
                    self.scrolling = false
                    self.autoScroll()
                }
            )
        }
    }
    
    func updatePlayingPosition() {
        CATransaction.begin()
        CATransaction.setValue(kCFBooleanTrue, forKey: kCATransactionDisableActions)
        playingLine!.transform = CATransform3DMakeTranslation(currentX(), 0, 0)
        CATransaction.commit()
    }
    
    func drawGridLayer() {
        let ctx = CGLayerGetContext(gridLayer)
        CGContextClearRect(ctx, self.bounds)
        
        CGContextSetLineWidth(ctx, 1.0)
        
        for i in 0...127 {
            let y = round(heightPerKey * CGFloat(i + 1)) + 0.5
            let color = (0 == (127 - i) % 12 ? gridColorCKey : gridColorKey).CGColor
            CGContextSetStrokeColorWithColor(ctx, color)
            CGContextMoveToPoint(ctx, 0, y)
            CGContextAddLineToPoint(ctx, self.bounds.size.width, y)
            CGContextStrokePath(ctx)
        }
        
        
        var tick = Int32(0 / widthPerTick)
        var tickTo =  Int32(self.bounds.size.width / widthPerTick)
        
        var location: Location = namidi.sequence.location(tick)
        location.t = 0
        
        tick = namidi.sequence.tick(location)
        var timeSign = namidi.sequence.timeSign(tick)
        
        while tick <= tickTo {
            let color = (1 == location.b ? gridColorMeasure : gridColorBeat).CGColor
            let x = round(CGFloat(tick + 120) * widthPerTick) + 0.5
            
            CGContextSetStrokeColorWithColor(ctx, color)
            CGContextMoveToPoint(ctx, x, 0)
            CGContextAddLineToPoint(ctx, x, self.bounds.size.height)
            CGContextStrokePath(ctx)
            
            if timeSign.numerator < ++location.b {
                location.b = 1
                ++location.m
                timeSign = namidi.sequence.timeSign(tick)
            }
            
            tick = namidi.sequence.tick(location)
        }
    }
    
    func drawNoteLayer() {
        let ctx = CGLayerGetContext(noteLayer)
        CGContextClearRect(ctx, self.bounds)
        
        for event in namidi.sequence.events {
            switch event {
            case is NoteEvent:
                let note: NoteEvent = event as! NoteEvent
                drawNote(ctx, note: note, active: false)
            default:
                break
            }
        }
    }
    
    func noteRect(note: NoteEvent) -> CGRect {
        let left = round(CGFloat(note.tick + 120) * widthPerTick) + 0.5
        let right = round(CGFloat(note.tick + 120 + Int(note.gatetime)) * widthPerTick) + 0.5
        let y = round(CGFloat(127 - note.noteNo + 1) * heightPerKey) + 0.5
        let length = right - left
        
        return NSMakeRect(left - min(5, length / 2), y - 5, length, 10)
    }
    
    func drawNote(ctx: CGContext, note: NoteEvent, active: Bool) {
        let rect:CGRect = noteRect(note)
        
        CGContextClearRect(ctx, rect)
        
        let lx = CGRectGetMinX(rect)
        let cx = CGRectGetMidX(rect)
        let rx = CGRectGetMaxX(rect)
        let by = CGRectGetMinY(rect)
        let cy = CGRectGetMidY(rect)
        let ty = CGRectGetMaxY(rect)
        let radius:CGFloat = min(rect.size.width / 2, 5.0)
        
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
        
        self.setNeedsDisplayInRect(rect)
    }
}
