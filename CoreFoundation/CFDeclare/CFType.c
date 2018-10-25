//
//  CFType.c
//  CoreFoundation
//
//  Created by Bill Sun on 2018/10/4.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include "CFType.h"

CFRange CFRangeMake(size_t location, size_t length)
{
    return (CFRange){.location = location, .length = length};
}

size_t CFMaxRange(CFRange range)
{
    return range.location + range.length;
}
