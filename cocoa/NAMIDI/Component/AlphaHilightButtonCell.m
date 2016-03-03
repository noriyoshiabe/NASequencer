//
//  AlphaHilightButtonCell.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "AlphaHilightButtonCell.h"

@implementation AlphaHilightButtonCell

- (void)highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    controlView.alphaValue = flag ? 0.5 : 1.0;
}

@end
