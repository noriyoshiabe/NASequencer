//
//  DocumentController.swift
//  NAMIDI
//
//  Created by abechan on 2015/02/15.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

import Cocoa

class DocumentController : NSWindowController, NAMidiProxyDelegate {
    
    @IBOutlet var pianoRollView: PianoRollView?
    @IBOutlet var statusView: StatusView?
    
    var namidi: NAMidiProxy?
    
    override func awakeFromNib() {
        window?.setFrameAutosaveName("sharedWindowFrame")
        
        let document = self.document? as Document
        namidi = document.namidi
        namidi?.addDelegate(self);
        namidi?.addDelegate(pianoRollView!)
        namidi?.addDelegate(statusView!)
    }
    
    override func windowDidLoad() {
        namidi?.parse()
    }
    
    func onError(namidi: NAMidiProxy!, typeId: UnsafePointer<Int8>, error: UInt32) {
        switch typeId.memory {
        case PlayerClass.typeID.memory:
            switch error {
            case PLAYER_ERROR_MIDI_CLIENT_NOT_AVAILABLE.value:
                let alert = NSAlert()
                alert.messageText = "MIDI out port is not available."
                alert.informativeText = "Enable IAC driver with Audio MIDI Settings and .\n" +
                                        "Then, setup Audio Unit for playing MIDI source.\n" +
                                        "\n" +
                                        "Sorry, but this annoying initial setup is needed until sound font engine is implemented."
                alert.runModal()
                close()
            default:
                break
            }
        default:
            break
        }
    }
}
