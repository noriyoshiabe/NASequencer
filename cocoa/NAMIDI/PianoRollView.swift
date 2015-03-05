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
    
    var playingPosition:CGFloat = 0
    var playingLine: CALayer?
    
    var scrolling:Bool = false
    
    var time:CFAbsoluteTime = CFAbsoluteTimeGetCurrent()
    var drawCount:Int = 0;
    
    override var flipped: Bool {
        return true
    }

    override func awakeFromNib() {
        playingLine = CALayer()
        playingLine!.backgroundColor = currentPositionColor.CGColor;
        self.layer!.addSublayer(playingLine)
    }
    
    override func drawRect(dirtyRect: NSRect) {
        let contextPtr = NSGraphicsContext.currentContext()!.graphicsPort
        let context = unsafeBitCast(contextPtr, CGContext.self)
        CGContextClipToRect(context, dirtyRect)
        
        if nil != gridLayer {
            CGContextDrawLayerAtPoint(context, CGPointZero, gridLayer)
        }
        
        if nil != self.context {
            drawNotes(context, dirtyRect: dirtyRect)
        }
        
        calcFPS();
    }
    
    func onParseFinished(namidi: NAMidiProxy, context: UnsafeMutablePointer<ParseContext>) {
        let try: ParseContextAdapter = ParseContextAdapter(contextRef: context)
        if try.hasError {
            return
        }
        self.context = try
        
        let width:CGFloat = CGFloat(self.context!.sequence.length + 240) * self.widthPerTick
        self.frame = NSMakeRect(0, 0, round(width), round((127 + 2) * self.heightPerKey))
        playingLine!.frame = NSMakeRect(0, 0, 1, self.bounds.size.height)
        
        let contextPtr = NSGraphicsContext.currentContext()!.graphicsPort
        let context = unsafeBitCast(contextPtr, CGContext.self)
        
        gridLayer = CGLayerCreateWithContext(context, self.bounds.size, nil)
        drawGridLayer()
        
        setNeedsDisplayInRect(self.bounds)
    }
    
    func currentX() -> CGFloat {
        let tick = nil != self.playerContext ? self.playerContext!.tick : 0
        return round(CGFloat(tick + 120) * widthPerTick)
    }
    
    func onPlayerContextChanged(namidi: NAMidiProxy!, context: UnsafeMutablePointer<PlayerContext>, playingNotes:CFArray!) {
        self.playerContext = context.memory
        
        setPlayingPosition(currentX())
        
        var size = CFArrayGetCount(playingNotes)
        for var i = 0; i < size; ++i {
            var ptr = CFArrayGetValueAtIndex(playingNotes, i)
            if ptr == UnsafeMutablePointer<Void>.null() {
                continue
            }
            
            var note:NoteEvent = unsafeBitCast(ptr, UnsafeMutablePointer<NoteEvent>.self).memory
            setNeedsDisplayInRect(noteRect(note))
        }
        
        autoScroll()
    }
    
    func autoScroll() {
        if scrolling {
            return
        }
        
        let toX: CGFloat = currentX()
        
        let parent:NSScrollView = self.superview?.superview? as NSScrollView
        if !parent.documentVisibleRect.intersects(CGRectMake(toX, 0, 0, self.bounds.size.height)) {
            scrolling = true
            
            NSAnimationContext.runAnimationGroup({ (context:NSAnimationContext!) -> Void in
                context.duration = 0.5
                parent.contentView.animator().setBoundsOrigin(CGPointMake(toX - round(120 * self.widthPerTick), parent.documentVisibleRect.origin.y))
                },
                completionHandler: {
                    self.scrolling = false
                    self.autoScroll()
            })
        }
    }
    
    func setPlayingPosition(x: CGFloat) {
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
        let ctx:CGContext = CGLayerGetContext(gridLayer)
        
        CGContextSetLineWidth(ctx, 1.0)
        
        for i in 0...127 {
            let y = round(heightPerKey * CGFloat(i + 1)) + 0.5
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
            let x = round(CGFloat(tick + 120) * widthPerTick) + 0.5
            
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
    
    func drawNotes(ctx: CGContext, dirtyRect: NSRect) {
        for event in self.context!.sequence.events {
            switch event.memory.__.clazz.memory.typeID {
            case NoteEventClass.typeID:
                let note:NoteEvent = unsafeBitCast(event, UnsafePointer<NoteEvent>.self).memory
                if dirtyRect.intersects(noteRect(note)) {
                    drawNote(ctx, note: note)
                }
            default:
                break
            }
        }
    }
    
    func noteRect(note: NoteEvent) -> CGRect {
        let left:CGFloat = round(CGFloat(note.__.tick + 120) * widthPerTick) + 0.5
        let right:CGFloat = round(CGFloat(note.__.tick + 120 + note.gatetime) * widthPerTick) + 0.5
        let y:CGFloat = round(CGFloat(127 - note.noteNo + 1) * heightPerKey) + 0.5
        
        return NSMakeRect(left - 5, y - 5, right - left, 10)
    }
    
    func drawNote(ctx: CGContext, note: NoteEvent) {
        let rect:CGRect = noteRect(note)
        
        let lx = CGRectGetMinX(rect)
        let cx = CGRectGetMidX(rect)
        let rx = CGRectGetMaxX(rect)
        let by = CGRectGetMinY(rect)
        let cy = CGRectGetMidY(rect)
        let ty = CGRectGetMaxY(rect)
        let radius:CGFloat = 5.0
        
        CGContextSetLineWidth(ctx, 1.0)
        
        var active:Bool = nil != playerContext
            && PLAYER_STATE_PLAYING.value == playerContext!.state.value
            && noteRect(note).intersects(CGRectMake(currentX() + 0.5, 0, 0, self.bounds.size.height))
        
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
