//
//  ExportWindowController.h
//  NAMIDI
//
//  Created by abechan on 3/1/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "FileRepresentation.h"

@class ExportWindowController;
@protocol ExportWindowControllerDelegate <NSObject>
- (void)exportControllerDidClose:(ExportWindowController *)controller;
@end

@interface ExportWindowController : NSWindowController
@property (weak, nonatomic) id <ExportWindowControllerDelegate> delegate;
@property (strong, nonatomic) FileRepresentation *file;
@property (strong, nonatomic) NSURL *outputUrl;
@end
