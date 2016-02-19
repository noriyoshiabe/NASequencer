//
//  LocationViewController.m
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "LocationViewController.h"
#import "Stub.h"

@interface LocationViewController ()
@property (weak) IBOutlet NSTextField *locationField;
@property (strong, nonatomic) PlayerRepresentation *player;
@end

@implementation LocationViewController

- (void)awakeFromNib
{
    self.view.wantsLayer = YES;
    self.view.layer.backgroundColor = [NSColor colorWithWhite:0 alpha:0.5].CGColor;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    _player = [[PlayerRepresentation alloc] init];
    
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
