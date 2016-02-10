//
//  PreferenceWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class PreferenceWindowController;
@protocol PreferenceWindowControllerDelegate <NSObject>
- (void) preferenceWindowControllerWillClose:(PreferenceWindowController *)controller;
@end

@protocol PreferenceViewController <NSObject>
@property (nonatomic, readonly) NSString *identifier;
@property (nonatomic, readonly) NSImage *toolbarItemImage;
@property (nonatomic, readonly) NSString *toolbarItemLabel;
@end

@interface PreferenceWindowController : NSWindowController
@property (weak, nonatomic) id<PreferenceWindowControllerDelegate> delegate;
- (void)addViewController:(NSViewController<PreferenceViewController> *)viewController;
@end
