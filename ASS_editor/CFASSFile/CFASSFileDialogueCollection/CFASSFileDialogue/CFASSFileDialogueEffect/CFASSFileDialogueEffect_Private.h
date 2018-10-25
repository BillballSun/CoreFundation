//
//  CFASSFileDialogueEffect_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/16.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueEffect_Private_h
#define CFASSFileDialogueEffect_Private_h

#include "CFASSFileDialogueEffect.h"

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateWithString(const wchar_t *beginPoint, const wchar_t *endPoint);

int CFASSFileDialogueEffectStoreStringResult(CFASSFileDialogueEffectRef effect, wchar_t *targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

#endif /* CFASSFileDialogueEffect_Private_h */
