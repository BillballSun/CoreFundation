//
//  CABidiPairedBracket.h
//  CoreFoundation
//
//  Created by Bill Sun on 2018/10/5.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CABidiPairedBracket_h
#define CABidiPairedBracket_h

#include <stdbool.h>

#include "CFType.h"

typedef enum CABidiPairedBracketType {
    CABidiPairedBracketTypeNone,
    CABidiPairedBracketTypeOpen,
    CABidiPairedBracketTypeClose
} CABidiPairedBracketType;

/**
 Get the Paired Bracket Type of the character, if it has pairedBracket, and parameter pairedBracket != NULL, also modify this too.

 @param character any valid character in unicode UTF32 format
 @param pairedBracket if not NULL and character has paired-bracket, it is returned
 @return if it does not have paired-bracket, CABidiPairedBracketTypeNone returned
 */
CABidiPairedBracketType CABidiGetPairedBracketType(UTF32Char character, UTF32Char *pairedBracket);

#endif /* CABidiPairedBracket_h */
