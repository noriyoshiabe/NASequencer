//
//  StatusBackgroundView.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/22.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class StatusBackgroundView : NSView {
    
    override func awakeFromNib() {
        layer!.backgroundColor = CGColorCreateGenericRGB(0, 0, 0, 0.5)
        layer!.cornerRadius = 20
        layer!.masksToBounds = true
    }
}