//
//  ConductorViewController.h
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NAMidiRepresentation.h"

@interface ConductorViewController : NSViewController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@end
