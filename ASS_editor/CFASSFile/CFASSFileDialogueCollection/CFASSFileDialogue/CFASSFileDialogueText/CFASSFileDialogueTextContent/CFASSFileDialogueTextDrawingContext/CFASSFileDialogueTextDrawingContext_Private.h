//
//  CFASSFileDialogueTextDrawingContext_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/13.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextDrawingContext_Private_h
#define CFASSFileDialogueTextDrawingContext_Private_h

#include "CFASSFileDialogueTextDrawingContext.h"

wchar_t *CFASSFileDialogueTextDrawingContextAllocateString(CFASSFileDialogueTextDrawingContextRef context);

CFASSFileDialogueTextDrawingContextRef CFASSFileDialogueTextDrawingContextCreateFromString(const wchar_t *string);

bool CFASSFileDialogueTextDrawingContextCheckValidation(CFASSFileDialogueTextDrawingContextRef context);

#endif /* CFASSFileDialogueTextDrawingContext_Private_h */
