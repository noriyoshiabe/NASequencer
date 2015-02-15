//
//  DocumentController.swift
//  namidi
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class DocumentController : NSWindowController {
    
    @IBOutlet var rootView: DocumentView?
    
    override func awakeFromNib() {
        window?.setFrameAutosaveName("sharedWindowFrame")
        
        var document = self.document? as Document
        
        println(document.model)
        println(rootView)
    }
}
