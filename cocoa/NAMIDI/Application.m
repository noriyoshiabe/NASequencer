//
//  Application.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Application.h"
#import "About.h"

@implementation Application

- (void)orderFrontStandardAboutPanel:(id)sender
{
    [[About sharedAbout] showWindow];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url
{
    [[NSDocumentController sharedDocumentController] openDocumentWithContentsOfURL:url display:YES completionHandler:^(NSDocument * _Nullable document, BOOL documentWasAlreadyOpen, NSError * _Nullable error) {
    }];
}

@end
