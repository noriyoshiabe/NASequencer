//
//  NSArray+Support.h
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSArray (Support)
- (NSArray *)mapObjectsUsingBlock:(id (^)(id obj))block;
@end
