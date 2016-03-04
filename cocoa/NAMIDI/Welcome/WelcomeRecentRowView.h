//
//  WelcomeRecentRowView.h
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "FileRepresentation.h"

@interface WelcomeRecentRowView : NSTableRowView
@property (strong, nonatomic) FileRepresentation *file;
@end
