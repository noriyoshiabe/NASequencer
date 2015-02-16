//
//  namidi-core-bridge.c
//  NAMIDI
//
//  Created by abechan on 2015/02/16.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#include "namidi-core-bridge.h"

NAMidi *NAMidiCreate()
{
    return NATypeNew(NAMidi);
}