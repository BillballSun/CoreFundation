//
//  CFType.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/10/1.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFType_h
#define CFType_h

#include <stdint.h>
#include <inttypes.h>

typedef uint32_t UTF32Char;
typedef uint16_t UTF16Char;
typedef uint8_t UTF8Char;

typedef UTF32Char UCS4Char;
typedef UTF16Char UCS2Char;

#define UTF32CharPRIu PRIu32
#define UTF32CharPRIo PRIo32
#define UTF32CharPRIx PRIx32
#define UTF32CharSCNu SCNu32
#define UTF32CharSCNo SCNo32
#define UTF32CharSCNx SCNx32

#define UTF16CharPRIu PRIu16
#define UTF16CharPRIo PRIo16
#define UTF16CharPRIx PRIx16
#define UTF16CharSCNu SCNu16
#define UTF16CharSCNo SCNo16
#define UTF16CharSCNx SCNx16

typedef struct CFRange
{
    size_t location;
    size_t length;
}CFRange;

#define CFRangeNotFound (CFRange){.location = SIZE_MAX, .length = 0}

CFRange CFRangeMake(size_t location, size_t length);

/**
 This is the index of the element next after the range
 This is also the amount of elements from the index 0 (not location) to the last element of this range

 @return return location(index) + length
 */
size_t CFMaxRange(CFRange range);

#endif /* CFType_h */
