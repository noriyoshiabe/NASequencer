//
//  NAMidiRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SequenceRepresentation.h"
#import "PlayerRepresentation.h"
#import "MixerRepresentation.h"
#import "ParseInfoRepresentation.h"
#import "FileRepresentation.h"

@class NAMidiRepresentation;
@protocol NAMidiRepresentationObserver <NSObject>
- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo;
@optional
- (void)namidiWillParse:(NAMidiRepresentation *)namidi fileChanged:(BOOL)fileChanged;
@end

@interface NAMidiRepresentation : NSObject
@property (readonly, nonatomic) SequenceRepresentation *sequence;
@property (readonly, nonatomic) PlayerRepresentation *player;
@property (readonly, nonatomic) MixerRepresentation *mixer;
@property (readonly, nonatomic) ParseInfoRepresentation *parseInfo;
@property (readonly, nonatomic) BOOL hasError;
@property (retain, nonatomic) FileRepresentation *file;
- (void)addObserver:(id<NAMidiRepresentationObserver>)observer;
- (void)removeObserver:(id<NAMidiRepresentationObserver>)observer;
- (void)parse;
@end
