//
//  FileDialogController.swift
//  namidi
//
//  Created by abechan on 2015/02/14.
//
//

import Foundation
import AppKit

class FileDialogController : NSObject {
    
    @IBAction func onSelectOpen(sender: AnyObject) {
        var openPanel = NSOpenPanel()
        openPanel.allowedFileTypes = ["namidi"]
        
        switch openPanel.runModal() {
        case NSOKButton:
            println(NSString(format: "### I wanna open %@", openPanel.URLs))
        default:
            break
        }
    }
}