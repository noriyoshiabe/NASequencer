//
//  WelcomeWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class WelcomeWindowController;
@protocol WelcomeWindowControllerDelegate <NSObject>
- (void)welcomeWindowControllerWillClose:(WelcomeWindowController *)controller;
@end

@interface WelcomeWindowController : NSWindowController
@property (weak, nonatomic) id<WelcomeWindowControllerDelegate> delegate;
@end
