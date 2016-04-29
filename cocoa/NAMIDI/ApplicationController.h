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
@property (readonly, nonatomic) BOOL needShowWelcome;
- (void)initialize;
- (void)finalize;
- (void)showWelcomeWindow;
- (void)closeWelcomeWindow;
- (void)showGettingStartedWindow;
- (void)showAboutWindow;
- (void)showPreferenceWindow;
- (void)showPreferenceWindowWithIdeintifier:(NSString *)identifier animate:(BOOL)animate;
- (NSArray *)allowedFileTypes;
- (NSArray *)allowedFileTypesInEditor;
- (void)openDocumentWithContentsOfURL:(NSURL *)url;
- (void)openDocument;
- (void)openDocumentInEditorWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler;
- (void)saveDocumentInEditorWindow:(NSWindow *)window filename:(NSString *)filename completion:(void (^)(NSURL *url))completionHandler;
- (void)createDocument;
- (void)createDocumentForWindow:(NSWindow *)window completion:(void (^)(NSURL *url))completionHandler;
- (void)openExampleDocument:(NSString *)fileType;
- (void)exportDocumentForWindow:(NSWindow *)window file:(FileRepresentation *)file;
@end

extern ApplicationController *AppController;
