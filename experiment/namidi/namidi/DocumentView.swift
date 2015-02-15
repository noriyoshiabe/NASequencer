//
//  DocumentView.swift
//  namidi
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Foundation
import AppKit

class DocumentView : NSView {

    override func awakeFromNib() {
        super.awakeFromNib()
        registerForDraggedTypes([NSFilenamesPboardType])
    }
    
    override func draggingEntered(sender: NSDraggingInfo) -> NSDragOperation {
        var pboard = sender.draggingPasteboard()
        var files:[String] = pboard.propertyListForType(NSFilenamesPboardType) as [String]
        return 1 == files.count && "namidi" == files[0].pathExtension ? NSDragOperation.Copy : NSDragOperation.None
    }
    
    override func performDragOperation(sender: NSDraggingInfo) -> Bool {
        var pboard = sender.draggingPasteboard()
        var files:[String] = pboard.propertyListForType(NSFilenamesPboardType) as [String]
        var docmuentControler = NSDocumentController.sharedDocumentController() as NSDocumentController
        docmuentControler.openDocumentWithContentsOfURL(NSURL(fileURLWithPath: files[0])!, display: true) { _ in }
        return true
    }
}