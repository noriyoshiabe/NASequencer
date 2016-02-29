//
//  ParseInfoRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ParseInfo.h"

@interface ParseErrorRepresentation : NSObject
@property (readonly, nonatomic) NSString *filepath;
@property (readonly, nonatomic) int line;
@property (readonly, nonatomic) int column;
@property (readonly, nonatomic) int code;
@property (readonly, nonatomic) NSArray *infos;
- (instancetype)initWithParseError:(ParseError *)error;
@end

@interface ParseInfoRepresentation : NSObject
@property (readonly, nonatomic) NSArray<NSString *> *filepaths;
@property (readonly, nonatomic) NSArray<ParseErrorRepresentation *> *errors;
- (instancetype)initWithParseInfo:(ParseInfo *)info;
@end
