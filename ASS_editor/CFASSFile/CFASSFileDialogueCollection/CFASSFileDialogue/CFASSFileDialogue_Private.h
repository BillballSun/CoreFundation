//
//  CFASSFileDialogue_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/17.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogue_Private_h
#define CFASSFileDialogue_Private_h

#include "CFASSFileDialogue.h"

#pragma mark - Read File

CFASSFileDialogueRef CFASSFileDialogueCreateWithString(const wchar_t *source);
/* source */
// begin with "Dialogue:", end with '\n' or '\0'
/* Discussion */
// this function should handle the empty source, that is point to '\n'

#pragma mark - Store Result

int CFASSFileDialogueStoreStringResult(CFASSFileDialogueRef dialogue, wchar_t *targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL
// this begin with "Dialogue:" end with '\n'


#endif /* CFASSFileDialogue_Private_h */
