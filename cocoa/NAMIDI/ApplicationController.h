//
//  ApplicationController.h
//  NAMIDI
//
//  Created by abechan on 2/11/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "FileRepresentation.h"

@interface ApplicationController : NSObject
+ (ApplicationController *)sharedInstance;
- (void)initialize;
- (void)showWelcomeWindow;
- (void)closeWelcomeWindow;
- (void)showGettingStartedWindow;
- (void)showAboutWindow;
- (void)showPreferenceWindow;
- (void)showEditorWindow;
- (void)showEditorWindowWithFile:(FileRepresentation *)file;
- (void)openDocumentWithContentsOfURL:(NSURL *)url;
- (void)openDocument;
- (void)openDocumentWithCompletion:(void (^)(NSURL *url))completionHandler;
- (void)createDocument;
@end
