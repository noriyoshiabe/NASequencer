//
//  LocationView.m
//  NAMIDI
//
//  Created by abechan on 2/20/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "LocationView.h"

@interface LocationView ()
@property (weak) IBOutlet NSTextField *locationField;
@end

@implementation LocationView

- (void)awakeFromNib
{
    self.wantsLayer = YES;
    self.layer.backgroundColor = [NSColor colorWithWhite:0 alpha:0.5].CGColor;
    self.layer.cornerRadius = 10.0;
    self.layer.masksToBounds = YES;
}

- (void)setPlayer:(PlayerRepresentation *)player
{
    _player = player;
    [self update];
}

- (void)update
{
    int64_t usec = _player.usec;
    Location location = _player.location;
    
    int min = (int)(usec / (1000 * 1000 * 60));
    int sec = (int)(usec / (1000 * 1000));
    int msec = (int)((usec / 1000) % 1000);
    
    _locationField.stringValue = [NSString stringWithFormat:@"%03d:%02d:%03d  %02d:%02d:%03d",
                                  location.m, location.b, location.t, min, sec, msec];
}

@end