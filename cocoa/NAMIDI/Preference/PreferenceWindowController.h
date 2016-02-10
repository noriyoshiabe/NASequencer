//
//  PreferenceWindowController.h
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol PreferenceViewController <NSObject>
@property (nonatomic, readonly) NSString *title;
@property (nonatomic, readonly) NSString *identifier;
@property (nonatomic, readonly) NSImage *toolbarItemImage;
@property (nonatomic, readonly) NSString *toolbarItemLabel;
@end

@interface PreferenceWindowController : NSWindowController
- (void)addViewController:(NSViewController<PreferenceViewController> *)viewController;
@end
