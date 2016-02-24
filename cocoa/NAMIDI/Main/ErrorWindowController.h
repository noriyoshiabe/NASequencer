//
//  ErrorWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/20/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NAMidiRepresentation.h"

@interface ErrorWindowController : NSWindowController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@end
