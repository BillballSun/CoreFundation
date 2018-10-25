//
//  CFUnicodeStringArray.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFUnicodeStringArray_h
#define CFUnicodeStringArray_h

#include <stdlib.h>
#include <stdbool.h>

typedef struct CFUnicodeStringArray *CFUnicodeStringArrayRef;

CFUnicodeStringArrayRef CFUnicodeStringArrayCopy(CFUnicodeStringArrayRef array);

CFUnicodeStringArrayRef CFUnicodeStringArrayCreateEmpty(void);

size_t CFUnicodeStringArrayGetLength(CFUnicodeStringArrayRef array);
/* Return Value */
// the length of the added string

size_t CFUnicodeStringArrayAddString(CFUnicodeStringArrayRef array, const wchar_t * string, bool transferOwnership);
/* Return Value */
// the length of the added string, return (size_t)-1 if failed

void CFUnicodeStringArrayRemoveStringAtIndex(CFUnicodeStringArrayRef array, size_t index);

size_t CFUnicodeStringArrayInsertStringAtIndex(CFUnicodeStringArrayRef array, const wchar_t * string, size_t index);

void CFUnicodeStringArraySwapStringPosition(CFUnicodeStringArrayRef array, size_t index1, size_t index2);

size_t CFUnicodeStringArrayAddStringWithEndChar(CFUnicodeStringArrayRef array, const wchar_t * string, const wchar_t endChar, bool *encounterEndChar);
/* endChar */
// treat as '\0'
/* Return Value */
// the length of the added string

const wchar_t *CFUnicodeStringArrayGetStringAtIndex(CFUnicodeStringArrayRef array, size_t index);

void CFUnicodeStringArrayDestory(CFUnicodeStringArrayRef array);

#endif /* CFUnicodeStringArray_h */
