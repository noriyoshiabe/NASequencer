//
//  LocationView.m
//  NAMIDI
//
//  Created by abechan on 2/20/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "LocationView.h"
#import "Color.h"

@interface LocationView () <PlayerRepresentationObserver>
@property (weak) IBOutlet NSTextField *locationField;
@end

@implementation LocationView

- (void)awakeFromNib
{
    self.wantsLayer = YES;
    self.layer.backgroundColor = [Color statusBackground].CGColor;
    self.layer.cornerRadius = 10.0;
    self.layer.masksToBounds = YES;
}

- (void)dealloc
{
    [_player removeObserver:self];
}

- (void)setPlayer:(PlayerRepresentation *)player
{
    _player = player;
    [_player addObserver:self];
    [self update];
}

- (void)update
{
    int64_t usec = _player.usec;
    Location location = _player.location;
    
    int min = (int)((usec / (1000 * 1000)) / 60);
    int sec = (int)((usec / (1000 * 1000)) % 60);
    int msec = (int)((usec / 1000) % 1000);
    
    _locationField.stringValue = [NSString stringWithFormat:@"%03d:%02d:%03d  %02d:%02d:%03d",
                                  location.m, location.b, location.t, min, sec, msec];
}

#pragma mark PlayerRepresentationObserver

- (void)player:(PlayerRepresentation *)player onNotifyClock:(int)tick usec:(int64_t)usec location:(Location)location
{
    [self update];
}

@end
