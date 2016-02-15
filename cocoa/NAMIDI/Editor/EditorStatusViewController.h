//
//  EditorStatusViewController.h
//  NAMIDI
//
//  Created by abechan on 2/15/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "FileRepresentation.h"

@class EditorStatusViewController;
@protocol EditorStatusViewControllerDelegate <NSObject>
- (void)statusViewController:(EditorStatusViewController *)controller didSelectFile:(FileRepresentation *)file;
@end

@interface EditorStatusViewController : NSViewController
@property (weak) IBOutlet NSTextField *potitionField;
@property (weak, nonatomic) id<EditorStatusViewControllerDelegate> delegate;
@property (strong, nonatomic) NSMutableArray *files;
- (void)selectFile:(FileRepresentation *)file;
@end
