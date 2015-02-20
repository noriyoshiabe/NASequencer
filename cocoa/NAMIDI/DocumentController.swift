//
//  DocumentController.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class DocumentController : NSWindowController {
    
    @IBOutlet var rootView: DocumentView?
    var namidi: NAMidi?
    
    override func awakeFromNib() {
        window?.setFrameAutosaveName("sharedWindowFrame")
        
        let document = self.document? as Document
        namidi = document.namidi
        namidi?.addObserverDelegate(rootView!)
    }
    
    override func windowDidLoad() {
        namidi?.parse()
        NSNotificationCenter.defaultCenter().addObserver(self, selector: "willCloseWindow:", name: NSWindowWillCloseNotification, object: nil)
    }
    
    func willCloseWindow(notification: NSNotification) {
        if self.window == notification.object as? NSWindow {
            namidi?.detatchObserver()
        }
    }
}
