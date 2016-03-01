//
//  FileRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/14/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "FileRepresentation.h"

#include <pwd.h>

@implementation FileRepresentation

- (instancetype)initWithURL:(NSURL *)url
{
    self = [super init];
    if (self) {
        _url = url;
    }
    return self;
}

- (BOOL)isEqual:(id)object
{
    FileRepresentation *file = object;
    return [_url isEqual:file.url];
}

- (NSString *)identifier
{
    return _url.absoluteString;
}

- (NSImage *)fileTypeIcon
{
    return [[NSWorkspace sharedWorkspace] iconForFileType:_url.lastPathComponent.pathExtension];
}

- (NSString *)filename
{
    return _url.lastPathComponent;
}

- (NSString *)directory
{
    return [_url.path.stringByDeletingLastPathComponent stringByReplacingOccurrencesOfString:NSUserHomeDirectory() withString:@"~"];
}

@end
