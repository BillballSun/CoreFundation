//
//  CFASSFileDialogueText.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueText_h
#define CFASSFileDialogueText_h

#include "CFASSFileDialogueTextContent.h"
#include "CFEnumerator.h"
#include "CFASSFileChange.h"

typedef struct CFASSFileDialogueText *CFASSFileDialogueTextRef;

#pragma mark - Copy/Destory

CFASSFileDialogueTextRef CFASSFileDialogueTextCopy(CFASSFileDialogueTextRef dialogueText);

void CFASSFileDialogueTextDestory(CFASSFileDialogueTextRef dialogueText);

void CFASSFileDialogueTextAddContent(CFASSFileDialogueTextRef dialogueText, CFASSFileDialogueTextContentRef content);

#pragma mark - Receive Change

void CFASSFileDialogueTextMakeChange(CFASSFileDialogueTextRef dialogueText, CFASSFileChangeRef change);

#pragma mark - Get Component

CFEnumeratorRef CFASSFileDialogueTextCreateEnumerator(CFASSFileDialogueTextRef dialogueText);

#endif /* CFASSFileDialogueText_h */
