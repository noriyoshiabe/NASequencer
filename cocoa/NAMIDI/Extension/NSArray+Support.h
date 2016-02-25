//
//  NSArray+Support.h
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSArray<__covariant ObjectType> (Support)
- (NSArray *)mapObjectsUsingBlock:(ObjectType (^)(ObjectType obj))block;
- (ObjectType)objectPassingTest:(BOOL (^)(ObjectType obj, NSUInteger idx, BOOL *stop))predicate;
@end
