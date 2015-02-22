//
//  StatusBackgroundView.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class StatusBackgroundView : NSView {
    
    let color: NSColor = NSColor(red: 0, green: 0, blue: 0, alpha: 0.5)
    
    override func drawRect(dirtyRect: NSRect) {
        var path:NSBezierPath = NSBezierPath(roundedRect: self.bounds, xRadius: 20, yRadius: 20)
        color.set()
        path.fill()
    }
}