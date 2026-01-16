//
//  FileRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/14/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <UniformTypeIdentifiers/UTType.h>

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

- (NSUInteger)hash
{
    return _url.hash;
}

- (NSImage *)fileTypeIcon
{
    if (@available(macOS 11.0, *)) {
        return [[NSWorkspace sharedWorkspace] iconForContentType:[UTType typeWithFilenameExtension:_url.pathExtension]];
    } else {
        return [[NSWorkspace sharedWorkspace] iconForFileType:_url.lastPathComponent.pathExtension];
    }
}

- (NSString *)filename
{
    return _url.lastPathComponent;
}

- (NSString *)directory
{
    return [_url.path.stringByDeletingLastPathComponent stringByReplacingOccurrencesOfString:NSUserHomeDirectory() withString:@"~"];
}

#pragma mark NSCopying

- (id)copyWithZone:(NSZone *)zone
{
    return [[FileRepresentation allocWithZone:zone] initWithURL:_url];
}

@end
