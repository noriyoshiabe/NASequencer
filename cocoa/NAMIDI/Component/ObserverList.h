//
//  ObserverList.h
//  NAMIDI
//
//  Created by abechan on 2/27/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ObserverList<ObjectType> : NSObject <NSFastEnumeration>
- (void)addObserver:(ObjectType)observer;
- (void)removeObserver:(ObjectType)observer;
@end
