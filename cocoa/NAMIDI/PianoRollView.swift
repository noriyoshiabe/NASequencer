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
    
    var gridLayer: CGLayer?
    
    var playingPosition: CGFloat = 0
    var playingLine: CALayer?
    
    var scrolling: Bool = false
    
    var time: CFAbsoluteTime = CFAbsoluteTimeGetCurrent()
    var drawCount: Int = 0;
    
    override var flipped: Bool {
        return true
    }

    override func awakeFromNib() {
        playingLine = CALayer()
        playingLine!.backgroundColor = currentPositionColor.CGColor;
        playingLine!.frame = NSMakeRect(0, 0, 1, round((127 + 2) * self.heightPerKey))
        self.layer!.addSublayer(playingLine)
    }
    
    override func drawRect(dirtyRect: NSRect) {
        let contextPtr = NSGraphicsContext.currentContext()!.graphicsPort
        let context = unsafeBitCast(contextPtr, CGContext.self)
        CGContextClipToRect(context, dirtyRect)
        
        if nil != gridLayer {
            CGContextDrawLayerAtPoint(context, CGPointZero, gridLayer)
        }
        
        if nil != namidi.sequence {
            drawNotes(context, dirtyRect: dirtyRect)
        }
        
        setPlayingPositionS(currentX())
        autoScroll()
        
        if (namidi.player.playing) {
            dispatch_async(dispatch_get_main_queue()) {
                self.needsDisplay = true
            }
        }
        
        calcFPS();
    }
    
    func namidi(namidi: NAMidi!, onParseFinish sequence: Sequence!) {
        dispatch_async(dispatch_get_main_queue()) {
            let width = CGFloat(namidi.sequence.length + 240) * self.widthPerTick
            self.frame = NSMakeRect(0, 0, round(width), round((127 + 2) * self.heightPerKey))
            
            let context = NSGraphicsContext.currentContext()?.CGContext
            
            self.gridLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
            self.drawGridLayer()
            
            self.needsDisplay = true
        }
    }
    
    func currentX() -> CGFloat {
        return round(CGFloat(namidi.player.tick + 120) * widthPerTick)
    }
    
    func namidi(namidi: NAMidi!, player: Player!, notifyEvent playerEvent: PlayerEvent) {
        needsDisplay = true
    }
    
    func namidi(namidi: NAMidi!, player: Player!, didSendNoteOn noteEvent: NoteEvent!) {
        dispatch_async(dispatch_get_main_queue()) {
            self.setNeedsDisplayInRect(self.noteRect(noteEvent))
        }
    }
    
    func namidi(namidi: NAMidi!, player: Player!, didSendNoteOff noteEvent: NoteEvent!) {
        dispatch_async(dispatch_get_main_queue()) {
            self.setNeedsDisplayInRect(self.noteRect(noteEvent))
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
    
    func setPlayingPositionS(x: CGFloat) {
        if playingPosition == x {
            return
        }
        
        CATransaction.begin()
        CATransaction.setValue(kCFBooleanTrue, forKey: kCATransactionDisableActions)
        playingLine!.transform = CATransform3DMakeTranslation(x, 0, 0)
        CATransaction.commit()
        
        playingPosition = x
    }
    
    func drawGridLayer() {
        let ctx = CGLayerGetContext(gridLayer)
        
        CGContextSetLineWidth(ctx, 1.0)
        
        for i in 0...127 {
            let y = round(heightPerKey * CGFloat(i + 1)) + 0.5
            let color = (0 == (127 - i) % 12 ? gridColorCKey : gridColorKey).CGColor
            CGContextSetStrokeColorWithColor(ctx, color)
            CGContextMoveToPoint(ctx, 0, y)
            CGContextAddLineToPoint(ctx, self.bounds.size.width, y)
            CGContextStrokePath(ctx)
        }
        
        let sequence = namidi.sequence
        
        var tick = Int32(0 / widthPerTick)
        var tickTo =  Int32(self.bounds.size.width / widthPerTick)
        
        var location: Location = namidi.sequence.location(tick)
        location.t = 0
        
        tick = sequence.tick(location)
        var timeSign = sequence.timeSign(tick)
        
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
                timeSign = sequence.timeSign(tick)
            }
            
            tick = sequence.tick(location)
        }
    }
    
    func drawNotes(ctx: CGContext, dirtyRect: NSRect) {
        for event in namidi.sequence.events {
            switch event {
            case is NoteEvent:
                let note: NoteEvent = event as! NoteEvent
                if dirtyRect.intersects(noteRect(note)) {
                    drawNote(ctx, note: note)
                }
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
    
    func drawNote(ctx: CGContext, note: NoteEvent) {
        let rect:CGRect = noteRect(note)
        
        let lx = CGRectGetMinX(rect)
        let cx = CGRectGetMidX(rect)
        let rx = CGRectGetMaxX(rect)
        let by = CGRectGetMinY(rect)
        let cy = CGRectGetMidY(rect)
        let ty = CGRectGetMaxY(rect)
        let radius:CGFloat = min(rect.size.width / 2, 5.0)
        
        CGContextSetLineWidth(ctx, 1.0)
        
        if namidi.player.playingNoteEvents.contains(note) {
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
    
    func calcFPS() {
        ++drawCount;
        var _time = CFAbsoluteTimeGetCurrent()
        if 1.0 < _time - time {
            println(String(format: "-FPS: %d", drawCount))
            time = _time
            drawCount = 0;
        }
    }
}
