//
//  DocumentController.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class DocumentController : NSWindowController, NAMidiObserver {
    
    @IBOutlet var pianoRollView: PianoRollView!
    @IBOutlet var statusView: StatusView!
    
    var namidi: NAMidi!
    
    override func awakeFromNib() {
        window?.setFrameAutosaveName("sharedWindowFrame")
        
        let document = self.document as! Document
        namidi = document.namidi
        namidi.addObserver(self)
        
        pianoRollView.namidi = namidi
        statusView.namidi = namidi
    }
    
    override func windowDidLoad() {
        let document = self.document as! Document
        namidi = document.namidi
        namidi.parse(document.fileUrl!.path)
        namidi.watch = true
    }
    
    func namidi(namidi: NAMidi!, onParseError filepath: String!, line: Int32, column: Int32, error: ParseError, info: UnsafePointer<Void>) {
        // TODO Alert etc...
    }
}
