//
//  DocumentView.swift
//  namidi
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

protocol DocumentViewDelegate : NSObjectProtocol {
    func draggingEntered(sender: DocumentView, draggingInfo: NSDraggingInfo) -> NSDragOperation
    func performDragOperation(sender: DocumentView, draggingInfo: NSDraggingInfo) -> Bool
}

class DocumentView : NSView {

    override func awakeFromNib() {
        super.awakeFromNib()
        registerForDraggedTypes([NSFilenamesPboardType])
    }
    
    override func draggingEntered(sender: NSDraggingInfo) -> NSDragOperation {
        return AppDelegate.sharedApplication().draggingEntered(self, draggingInfo: sender)
    }
    
    override func performDragOperation(sender: NSDraggingInfo) -> Bool {
        return AppDelegate.sharedApplication().performDragOperation(self, draggingInfo: sender)
    }
}