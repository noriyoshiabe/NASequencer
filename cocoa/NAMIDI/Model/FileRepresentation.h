//
//  FileRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/14/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface FileRepresentation : NSObject
@property (readonly) NSString *identifier;
@property (readonly) NSURL *url;
@property (readonly) NSImage *fileTypeIcon;
@property (readonly) NSString *filename;
@property (readonly) NSString *directory;

- (instancetype)initWithURL:(NSURL *)url;
@end
