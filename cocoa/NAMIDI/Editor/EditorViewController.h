//
//  EditorViewController.h
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "FileRepresentation.h"

@class EditorViewController;
@protocol EditorViewControllerDelegate <NSObject>
- (void)editorViewController:(EditorViewController *)controller didUpdateLine:(NSUInteger)line column:(NSUInteger)column;
- (void)editorViewController:(EditorViewController *)controller didPerformCloseAction:(id)sender;
@end

@interface EditorViewController : NSViewController
@property (unsafe_unretained) IBOutlet NSTextView *textView;
@property (weak, nonatomic) id<EditorViewControllerDelegate> delegate;
@property (strong, nonatomic) FileRepresentation *file;
@end
