//
//  EditorWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "FileRepresentation.h"

@interface EditorWindowController : NSWindowController
- (void)addFileRepresentation:(FileRepresentation *)file;
- (BOOL)closeFilesWithConfirmation;
@end
