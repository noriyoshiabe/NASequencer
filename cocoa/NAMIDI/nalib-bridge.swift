//
//  nalib-bridge.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/17.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation

func NARelease(_self: COpaquePointer)
{
    NARelease(UnsafeMutablePointer<Void>(_self))
}