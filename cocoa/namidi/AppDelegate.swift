//
//  AppDelegate.swift
//  namidi
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate, DocumentViewDelegate {

    func applicationDidFinishLaunching(aNotification: NSNotification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(aNotification: NSNotification) {
        // Insert code here to tear down your application
    }

    func applicationShouldOpenUntitledFile(sender: NSApplication) -> Bool {
        return false
    }
    
    func application(sender: NSApplication, openFile filename: String) -> Bool {
        let docmuentControler = NSDocumentController.sharedDocumentController() as NSDocumentController
        docmuentControler.openDocumentWithContentsOfURL(NSURL(fileURLWithPath: filename)!, display: true) { _ in }
        return true
    }
    
    // Instant method
    class func sharedApplication() -> AppDelegate {
        return NSApplication.sharedApplication().delegate as AppDelegate
    }

    // DocumentViewDelegate
    
    func draggingEntered(sender: DocumentView, draggingInfo: NSDraggingInfo) -> NSDragOperation {
        var pboard = draggingInfo.draggingPasteboard()
        var files:[String] = pboard.propertyListForType(NSFilenamesPboardType) as [String]
        return 1 == files.count && "namidi" == files[0].pathExtension ? NSDragOperation.Copy : NSDragOperation.None
    }
    
    func performDragOperation(sender: DocumentView, draggingInfo: NSDraggingInfo) -> Bool {
        let pboard = draggingInfo.draggingPasteboard()
        let files:[String] = pboard.propertyListForType(NSFilenamesPboardType) as [String]
        let docmuentControler = NSDocumentController.sharedDocumentController() as NSDocumentController
        docmuentControler.openDocumentWithContentsOfURL(NSURL(fileURLWithPath: files[0])!, display: true) { _ in }
        return true
    }
    
    // IBAction
    
    @IBAction func onPlayPause(sender: AnyObject) {
        let docmuentControler = NSDocumentController.sharedDocumentController() as NSDocumentController
        
        let alert = NSAlert()
        alert.messageText = "Unimplemented!"
        alert.informativeText = "Play/Pause " + docmuentControler.currentDocument!.fileURL!!.path!
        alert.runModal()
    }
    
    @IBAction func onRewind(sender: AnyObject) {
        let docmuentControler = NSDocumentController.sharedDocumentController() as NSDocumentController
        
        let alert = NSAlert()
        alert.messageText = "Unimplemented!"
        alert.informativeText = "Rewind" + docmuentControler.currentDocument!.fileURL!!.path!
        alert.runModal()
    }
}

