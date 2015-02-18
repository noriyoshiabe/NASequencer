//
//  NAMidi.swift
//  NAMIDI
//
//  Created by 張阿部 on 2015/02/18.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation

class NAMidi {
    private let namidi: COpaquePointer
    
    init() {
        namidi = NAMidiCreate()
    }
    
    deinit {
        NARelease(UnsafeMutablePointer<Void>(namidi))
    }
    
    func setFile(url: NSURL) {
        NAMidiSetFile(namidi, url.path! as CFString)
    }
    
    func parse() {
        NAMidiParse(namidi)
    }
    
    func play() {
        NAMidiPlay(namidi)
    }
    
    func stop() {
        NAMidiStop(namidi)
    }
    
    func playPause() {
        NAMidiPlayPause(namidi)
    }
    
    func rewind() {
        NAMidiRewind(namidi)
    }
    
    func forward() {
        NAMidiForward(namidi)
    }
    
    func backward() {
        NAMidiBackward(namidi)
    }
}