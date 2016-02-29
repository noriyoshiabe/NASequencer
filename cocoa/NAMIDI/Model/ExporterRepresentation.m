//
//  ExporterRepresentation.m
//  NAMIDI
//
//  Created by abechan on 3/1/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ExporterRepresentation.h"

@interface ExporterRepresentation () {
    Exporter *_exporter;
}

- (void)onParseFinish:(ParseInfo *)info;
- (void)onProgress:(int)progress;
@end

static void onParseFinish(void *receiver, ParseInfo *info)
{
    ExporterRepresentation *exporter = (__bridge ExporterRepresentation *)receiver;
    [exporter onParseFinish:info];
}

static void onProgress(void *receiver, int progress)
{
    ExporterRepresentation *exporter = (__bridge ExporterRepresentation *)receiver;
    [exporter onProgress:progress];
}

static ExporterObserverCallbacks callbacks = {onParseFinish, onProgress};

@implementation ExporterRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _exporter = ExporterCreate(&callbacks, (__bridge void *)self);
    }
    return self;
}

- (void)dealloc
{
    if (_exporter) {
        ExporterDestroy(_exporter);
    }
}

- (void)onParseFinish:(ParseInfo *)info
{
    if ([_delegate respondsToSelector:@selector(exporter:onParseFinish:)]) {
        [_delegate exporter:self onParseFinish:[[ParseInfoRepresentation alloc] initWithParseInfo:info]];
    }
}

- (void)onProgress:(int)progress
{
    if ([_delegate respondsToSelector:@selector(exporter:onProgress:)]) {
        [_delegate exporter:self onProgress:progress];
    }
}

- (ExporterError)export:(NSString *)filepath output:(NSString *)output
{
    return ExporterExport(_exporter, filepath.UTF8String, output.UTF8String);
}

@end
