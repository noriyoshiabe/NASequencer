//
//  Application.h
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface Application : NSApplication
- (void)openDocumentWithContentsOfURL:(NSURL *)url;
- (void)openDocument;
@end
