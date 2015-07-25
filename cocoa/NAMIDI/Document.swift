//
//  Document.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class Document: NSDocument {

    let namidi: NAMidi
    let mixer: Mixer
    var fileUrl: NSURL?
    
    override init() {
        namidi = NAMidi()
        mixer = Mixer()
        namidi.player.mixer = mixer
        super.init()
    }
    
    override func readFromURL(url: NSURL, ofType typeName: String, error outError: NSErrorPointer) -> Bool {
        fileUrl = url
        return true
    }

    override func makeWindowControllers() {
        addWindowController(DocumentController(windowNibName: "Document"))
    }
}