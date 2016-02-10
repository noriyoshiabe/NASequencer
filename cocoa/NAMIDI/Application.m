//
//  Application.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Application.h"
#import "About.h"
#import "Welcome.h"

@implementation Application

- (void)orderFrontStandardAboutPanel:(id)sender
{
    [[About sharedInstance] showWindow];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url
{
    [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:url display:YES completionHandler:^(NSDocument * _Nullable document, BOOL documentWasAlreadyOpen, NSError * _Nullable error) {
    }];
}

- (void)openDocument
{
    NSOpenPanel *openPanel = [[NSOpenPanel alloc] init];
    
    [openPanel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            [[Welcome sharedInstance] closeWindow];
            [self openDocumentWithContentsOfURL:[openPanel URL]];
        }
    }];
}

@end
