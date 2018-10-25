//
//  UTF32String.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/10/1.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef UTF32String_h
#define UTF32String_h

#include "CFType.h"

typedef struct UTF32String *UTF32StringRef;

UTF32Char UTF32StringGetCharAtIndex(UTF32StringRef string, size_t index);

size_t UTF32StringGetLength(UTF32StringRef string);

UTF32StringRef UTF32StringCreateWithUTF8String(const UTF8Char *string);

UTF32StringRef UTF32StringCreateWithCharacterArray(UTF32Char *array, size_t length);

void UTF32StringDestory(UTF32StringRef string);

UTF32StringRef UTF32StringCopy(UTF32StringRef from);

#endif /* UTF32String_h */
