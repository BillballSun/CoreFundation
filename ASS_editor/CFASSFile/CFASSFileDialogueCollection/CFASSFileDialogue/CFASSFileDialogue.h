//
//  CFASSFileDialogue.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogue_h
#define CFASSFileDialogue_h

#include "CFASSFileChange.h"

typedef struct CFASSFileDialogue *CFASSFileDialogueRef;

#pragma mark - Copy/Destory

CFASSFileDialogueRef CFASSFileDialogueCopy(CFASSFileDialogueRef dialogue);

void CFASSFileDialogueDestory(CFASSFileDialogueRef dialogue);

#pragma mark - Receive Change

void CFASSFileDialogueMakeChange(CFASSFileDialogueRef dialogue, CFASSFileChangeRef change);

#endif /* CFASSFileDialogue_h */
