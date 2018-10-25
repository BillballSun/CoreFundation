//
//  CFASSFileDialogueTextContent_Prvivate.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContent_Private_h
#define CFASSFileDialogueTextContent_Private_h

#include "CFASSFileDialogueTextContent.h"

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithString(CFASSFileDialogueTextContentType type,
                                                                             const wchar_t *data,
                                                                             const wchar_t *endPoint);
/* endPoint */
// this must be the character before the '\n' or '\0', very important
// if this is an empty text, this should be called with CFASSFileDialogueTextContentCreateEmptyString

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateEmptyString(void);

int CFASSFileDialogueTextContentStoreStringResult(CFASSFileDialogueTextContentRef textContent, wchar_t * targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

#endif /* CFASSFileDialogueTextContent_Private_h */
