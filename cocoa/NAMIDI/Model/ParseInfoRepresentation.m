//
//  ParseInfoRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ParseInfoRepresentation.h"

@interface ParseErrorRepresentation () {
    ParseError *_error;
    NSMutableArray *_infos;
}

@end

@implementation ParseErrorRepresentation

- (instancetype)initWithParseError:(ParseError *)error
{
    self = [super init];
    if (self) {
        _error = error;
        _filepath = [NSString stringWithUTF8String:_error->location.filepath];
        _infos = [NSMutableArray array];
        NAIterator *iterator = NAArrayGetIterator(_error->infos);
        while (iterator->hasNext(iterator)) {
            [_infos addObject:[NSString stringWithUTF8String:iterator->next(iterator)]];
        }
    }
    return self;
}


- (int)line
{
    return _error->location.line;
}

- (int)column
{
    return _error->location.column;
}

- (int)code
{
    return _error->code;
}

@end

@interface ParseInfoRepresentation () {
    ParseInfo *_info;
    NSMutableArray *_filepaths;
    NSMutableArray *_errors;
}

@end

@implementation ParseInfoRepresentation

- (instancetype)initWithParseInfo:(ParseInfo *)info
{
    self = [super init];
    if (self) {
        _info = info;
        
        NAIterator *iterator;
        
        _filepaths = [NSMutableArray array];
        iterator = NAArrayGetIterator(_info->filepaths);
        while (iterator->hasNext(iterator)) {
            [_filepaths addObject:[NSString stringWithUTF8String:iterator->next(iterator)]];
        }
        
        _errors = [NSMutableArray array];
        iterator = NAArrayGetIterator(_info->errors);
        while (iterator->hasNext(iterator)) {
            [_errors addObject:[[ParseErrorRepresentation alloc] initWithParseError:iterator->next(iterator)]];
        }
        
        ParseInfoRetain(_info);
    }
    return self;
}

- (void)dealloc
{
    if (_info) {
        ParseInfoRelease(_info);
    }
}

@end
