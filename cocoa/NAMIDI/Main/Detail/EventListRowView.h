//
//  EventListRowView.h
//  NAMIDI
//
//  Created by abechan on 2/24/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface EventListRowView : NSTableRowView
@property (strong, nonatomic) NSString *location;
@property (strong, nonatomic) NSString *type;
@property (strong, nonatomic) NSString *channel;
@property (strong, nonatomic) NSString *note;
@property (strong, nonatomic) NSString *gatetime;
@property (strong, nonatomic) NSString *velocity;
@property (strong, nonatomic) NSString *other;
@end
