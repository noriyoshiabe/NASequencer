//
//  MainView.swift
//  namidi
//
//  Created by abechan on 2015/02/14.
//
//

import Foundation
import AppKit

class MainView : NSView {
    
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
        println(NSString(format: "### I wanna open %@", files[0]))
        return true
    }
}