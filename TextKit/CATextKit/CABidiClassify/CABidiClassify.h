//
//  CABidiClassify.h
//  TextKit
//
//  Created by Bill Sun on 2018/10/2.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CABidiClassify_h
#define CABidiClassify_h

#include <stdbool.h>
#include "CFType.h"

typedef enum CABidiType {
    
    /* strong */
    CABidiTypeL,   // Left to Right
    CABidiTypeR,   // Right to Left
    CABidiTypeAL,  // Right to Left Arabic
    
    /* weak */
    CABidiTypeEN,  // European Number
    CABidiTypeES,  // European Number Separator
    CABidiTypeET,  // European Number Terminator
    CABidiTypeAN,  // Arabic Number
    CABidiTypeCS,  // Common Number Seperator
    CABidiTypeNSM, // Nonspacing Mark
    CABidiTypeBN,  // Boundary Neutral
    
    /* Neutral */
    CABidiTypeB,   // Paragraph Seperator
    CABidiTypeS,   // Segment Seperator
    CABidiTypeWS,  // Whitespace
    CABidiTypeON,  // Other Neutrals
    
    /* Explicit Formatting */
    CABidiTypeLRE, // Left to Right Embedding
    CABidiTypeLRO, // Left to Right Override
    CABidiTypeRLE, // Right to Left Embedding
    CABidiTypeRLO, // Right to Left Override
    CABidiTypePDF, // Pop Directional Format
    CABidiTypeLRI, // Left to Right Isolate
    CABidiTypeRLI, // Right to Left Isolate
    CABidiTypeFSI, // First Strong Isolate
    CABidiTypePDI  // Pop Directional Isolate
} CABidiType;

typedef enum CABidiCategory {
    CABidiCategoryStrong,
    CABidiCategoryWeak,
    CABidiCategoryNeutral,
    CABidiCategoryExplicitFormatting
}CABidiCategory;

CABidiType CABidiClassifyTypeForCharacter(UTF32Char character);
CABidiCategory CABidiClassifyCategoryForCharacter(UTF32Char character);
CABidiCategory CABidiClassifyCategoryForType(CABidiType type);

bool isExplicitDirectionalIsolates(CABidiType type);
bool isExplicitDirectionalEmbeddings(CABidiType type);

#endif /* CABidiClassify_h */
