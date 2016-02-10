//
//  AboutWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AboutWindowTextLink : NSTextField
@end

@class AboutWindowController;
@protocol AboutWindowControllerDelegate <NSObject>
- (void)aboutWindowControllerWillClose:(AboutWindowController *)controller;
@end

@interface AboutWindowController : NSWindowController
@property (weak, nonatomic) id<AboutWindowControllerDelegate> delegate;
@end
