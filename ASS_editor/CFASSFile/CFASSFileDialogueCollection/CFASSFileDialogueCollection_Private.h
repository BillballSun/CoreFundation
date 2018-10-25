//
//  CFASSFileDialogueCollection_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueCollection_Private_h
#define CFASSFileDialogueCollection_Private_h

#include <wchar.h>

#include "CFASSFileDialogueCollection.h"

#pragma mark - Read File

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCreateWithUnicodeFileContent(const wchar_t *content);

#pragma mark - Allocate Result

wchar_t *CFASSFileDialogueCollectionAllocateFileContent(CFASSFileDialogueCollectionRef dialogueCollection);

#pragma mark - Register Callbacks

int CFASSFileDialogueCollectionRegisterAssociationwithFile(CFASSFileDialogueCollectionRef collection, CFASSFileRef assFile);
/* Return */
// return zero means success, -1 means failed

#endif /* CFASSFileDialogueCollection_Private_h */
