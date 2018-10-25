//
//  CFASSFileDialogueText_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueText_Private_h
#define CFASSFileDialogueText_Private_h

#pragma mark - Read File

CFASSFileDialogueTextRef CFASSFileDialogueTextCreateWithString(const wchar_t *source);
/* source */
// begin with current text filed
/* Discussion */
// this function should handle the empty source, that is point to '\n' or '\0'

#pragma mark - Store Result

int CFASSFileDialogueTextStoreStringResult(CFASSFileDialogueTextRef text, wchar_t * targetPoint);
/* targetPoint */
// this could be NULL
/* Return */
// if anything went wrong, function return -1
// if targetPoint is NULL, it will not try to store string in it
// return will be the store string Length if targetPoint is long enough even is NULL
// this will count until the '\n', so it will store the last '\n'

#endif /* CFASSFileDialogueText_Private_h */
