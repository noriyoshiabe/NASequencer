//
//  DocumentController.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class DocumentController : NSWindowController {
    
    @IBOutlet var pianoRollView: PianoRollView?
    @IBOutlet var statusView: StatusView?
    
    var namidi: NAMidiSW?
    
    override func awakeFromNib() {
        window?.setFrameAutosaveName("sharedWindowFrame")
        
        let document = self.document? as Document
        namidi = document.namidi
        namidi?.addObserverDelegate(pianoRollView!)
        namidi?.addObserverDelegate(statusView!)
    }
    
    override func windowDidLoad() {
        namidi?.parse()
    }
}
