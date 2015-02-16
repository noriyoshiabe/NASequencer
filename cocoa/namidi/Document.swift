//
//  Document.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class Document: NSDocument {

    var model: String?
    
    override init() {
        super.init()
        // Add your subclass-specific initialization here.
    }
    
    override func readFromURL(url: NSURL, ofType typeName: String, error outError: NSErrorPointer) -> Bool {
        model = url.path
        return true
    }

    override func makeWindowControllers() {
        addWindowController(DocumentController(windowNibName: "Document"))
    }
}

