//
//  MainWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NAMidiRepresentation.h"

@interface MainWindowController : NSWindowController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
- (void)showEditorWindow;
@end
