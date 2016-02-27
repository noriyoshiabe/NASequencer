//
//  ObserverList.m
//  NAMIDI
//
//  Created by abechan on 2/27/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ObserverList.h"
#import "NAArray.h"

@interface ObserverList () {
    NAArray *_array;
}

@end

@implementation ObserverList

- (instancetype)init
{
    self = [super init];
    if (self) {
        _array = NAArrayCreate(4, NADescriptionAddress);
    }
    return self;
}

- (void)dealloc
{
    NAArrayDestroy(_array);
}

- (void)addObserver:(id)observer
{
    NAArrayAppend(_array, (__bridge void *)observer);
}

- (void)removeObserver:(id)observer
{
    int index = NAArrayFindFirstIndex(_array, (__bridge const void *)observer, NAArrayAddressComparator);
    NAArrayRemoveAt(_array, index);
}

#pragma mark NSFastEnumeration

- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(__unsafe_unretained id  _Nonnull *)buffer count:(NSUInteger)len
{
    if (0 == state->state) {
        state->extra[0] = NAArrayCount(_array) - 1;
    }
    
    long bufferIndex = 0;
    long index = state->state;
    
    while (bufferIndex < len) {
        if (0 > (int)state->extra[0] - index) {
            break;
        }
        
        buffer[bufferIndex++] = (__bridge id)NAArrayGetValueAt(_array, (int)(state->extra[0] - index++));
    }
    
    state->state = index;
    state->itemsPtr = buffer;
    state->mutationsPtr = (unsigned long*)(__bridge void*)self;
    
    return bufferIndex;
}

@end
