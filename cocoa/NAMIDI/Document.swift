//
//  Document.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class Document: NSDocument {

    var namidi: COpaquePointer?
    
    override init() {
        super.init()
        // Add your subclass-specific initialization here.
    }
    
    override func readFromURL(url: NSURL, ofType typeName: String, error outError: NSErrorPointer) -> Bool {
        namidi = NAMidiCreate()
        NAMidiSetFile(namidi!, url.path! as CFString)
        NAMidiStart(namidi!)
        return true
    }

    override func makeWindowControllers() {
        addWindowController(DocumentController(windowNibName: "Document"))
    }
    
    override func canCloseDocumentWithDelegate(delegate: AnyObject, shouldCloseSelector: Selector, contextInfo: UnsafeMutablePointer<Void>) {
        NARelease(namidi!)
        super.canCloseDocumentWithDelegate(delegate, shouldCloseSelector: shouldCloseSelector, contextInfo: contextInfo)
    }
}

