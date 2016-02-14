//
//  Document.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Document.h"
#import "MainWindowController.h"

@interface Document ()

@end

@implementation Document

- (instancetype)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
    }
    return self;
}

- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)typeName error:(NSError * _Nullable __autoreleasing *)outError
{
    NSLog(@"## %@", [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:nil]);
    return YES;
}

- (void)makeWindowControllers
{
    MainWindowController *mainWC = [[MainWindowController alloc] init];
    [self addWindowController: mainWC];
}

@end
