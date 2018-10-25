//
//  CFASSFileDialogueTextContentOverride_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverride_Private_h
#define CFASSFileDialogueTextContentOverride_Private_h

#include <stdbool.h>
#include <stddef.h>

#include "CFASSFileDialogueTextContentOverride.h"

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithString(const wchar_t *string, bool isIncludeBraces);
/* string */
// this should be an string than only contain override info, if isIncludeBraces is false
// this could contain other info, but string is point to '{', if the isIncludeBraces is true
// WARNING: if isIncludeBraces is true, it must endwith '}'

int CFASSFileDialogueTextContentOverrideStoreStringResult(CFASSFileDialogueTextContentOverrideRef override, wchar_t * targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

#endif /* CFASSFileDialogueTextContentOverride_Private_h */
