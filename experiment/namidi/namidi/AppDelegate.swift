//
//  AppDelegate.swift
//  namidi
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {



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
        var docmuentControler = NSDocumentController.sharedDocumentController() as NSDocumentController
        docmuentControler.openDocumentWithContentsOfURL(NSURL(fileURLWithPath: filename)!, display: true) { _ in }
        return true
    }
}

