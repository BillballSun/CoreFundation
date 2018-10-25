//
//  CABLGenerator.h
//  TextSystem
//
//  Created by Bill Sun on 2018/9/30.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CABLGenerator_h
#define CABLGenerator_h

#include "UTF32String.h"

typedef struct CABLGenerator *CABLGeneratorRef;

typedef int_fast8_t CABidiLevel;

#pragma mark - Step 01 create CABLGenerator

/**
 The designated initializer for CABLGenerator

 @param string the length of string should not be zero, otherwise return NULL (it is retained by CABLGenerator)
 @return the resolved level of the string
 */
CABLGeneratorRef CABLGeneratorCreateWithString(UTF32StringRef string);

#pragma mark - Step 02 get Paragraph Information

size_t CABLGeneratorGetParagraphAmount(CABLGeneratorRef generator);
CFRange CABLGeneratorGetParagraphRange(CABLGeneratorRef generator, size_t paragraphIndex);

#pragma mark - Step 03 query for display order

/**
 This use the resolved embedding level to determine the display order of each line
 If paragraphRange or lineRange got zero length, or if lineRange does not fall in paragraph range, or some beyond the string range, NULL returned
 @param generator if NULL, NULL returned
 @param paragraphRange it is got by CABLGeneratorGetParagraphRange, specify nonsense range will process accordingly without warning
 @param lineRange this must inside the paragraph range
 @return a list of index(In String) in display order, the size is the length of lineRange, and it is your responsibility to call free <stdlib.h> at proper time
 */
size_t *CABLGeneratorDisplayLine(CABLGeneratorRef generator, CFRange paragraphRange, CFRange lineRange);

#pragma mark - Additional ask for internal information

void CABLGeneratorDestory(CABLGeneratorRef generator);

#endif /* CABLGenerator_h */
