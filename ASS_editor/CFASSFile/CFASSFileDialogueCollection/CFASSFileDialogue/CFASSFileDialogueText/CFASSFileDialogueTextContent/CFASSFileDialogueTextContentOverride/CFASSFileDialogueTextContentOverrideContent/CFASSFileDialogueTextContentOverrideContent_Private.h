//
//  CFASSFileDialogueTextContentOverrideContent_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/15.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverrideContent_Private_h
#define CFASSFileDialogueTextContentOverrideContent_Private_h

#include <wchar.h>

#include "CFASSFileDialogueTextContentOverrideContent.h"

#pragma mark - Read File

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCreateWithString(const wchar_t *contentString, const wchar_t *endPoint);
/* endPoint */
// points to the last possible content, this could be NULL
// for example, \move (1, 1), then the endPoint points to ')' is correct

#pragma mark - Store Result

int CFASSFileDialogueTextContentOverrideContentStoreStringResult(CFASSFileDialogueTextContentOverrideContentRef overrideContent, wchar_t * targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL

#endif /* CFASSFileDialogueTextContentOverrideContent_Private_h */
