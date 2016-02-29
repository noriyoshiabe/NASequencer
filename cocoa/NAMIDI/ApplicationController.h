//
//  ApplicationController.h
//  NAMIDI
//
//  Created by abechan on 2/11/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "FileRepresentation.h"

@interface ApplicationController : NSObject
- (void)initialize;
- (void)showWelcomeWindow;
- (void)closeWelcomeWindow;
- (void)showGettingStartedWindow;
- (void)showAboutWindow;
- (void)showPreferenceWindow;
- (NSArray *)allowedFileTypes;
- (void)openDocumentWithContentsOfURL:(NSURL *)url;
- (void)openDocument;
- (void)openDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler;
- (void)saveDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler;
- (void)createDocument;
- (void)createDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler;
- (void)exportDocumentForWindow:(NSWindow *)window file:(FileRepresentation *)file;
@end

extern ApplicationController *AppController;
