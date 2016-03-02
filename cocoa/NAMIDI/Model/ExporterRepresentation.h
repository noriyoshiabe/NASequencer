//
//  ExporterRepresentation.h
//  NAMIDI
//
//  Created by abechan on 3/1/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ParseInfoRepresentation.h"
#import "Exporter.h"

@class ExporterRepresentation;
@protocol ExporterRepresentationDelegate <NSObject>
@optional
- (void)exporter:(ExporterRepresentation *)exporter onParseFinish:(ParseInfoRepresentation *)info;
- (void)exporter:(ExporterRepresentation *)exporter onProgress:(int)progress;
@end

@interface ExporterRepresentation : NSObject
@property (assign, nonatomic) id<ExporterRepresentationDelegate> delegate;
@property (retain, nonatomic) NSString *includePath;
- (ExporterError)export:(NSString *)filepath output:(NSString *)output;
@end
