//
//  CFASSFileDialogueTextContentOverride.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverride_h
#define CFASSFileDialogueTextContentOverride_h

#include "CFASSFileDialogueTextContentOverrideContent.h"
#include "CFEnumerator.h"
#include "CFASSFileChange.h"

typedef struct CFASSFileDialogueTextContentOverride *CFASSFileDialogueTextContentOverrideRef;

#pragma mark - Create/Copy/Destory

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithContent(CFASSFileDialogueTextContentOverrideContentRef content);

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCopy(CFASSFileDialogueTextContentOverrideRef override);

void CFASSFileDialogueTextContentOverrideDestory(CFASSFileDialogueTextContentOverrideRef override);

void CFASSFileDialogueTextContentOverrideAddContent(CFASSFileDialogueTextContentOverrideRef override, CFASSFileDialogueTextContentOverrideContentRef content);

#pragma mark - Receive Change

void CFASSFileDialogueTextContentOverrideMakeChange(CFASSFileDialogueTextContentOverrideRef override, CFASSFileChangeRef change);

#pragma mark - Get Component

CFEnumeratorRef CFASSFileDialogueTextContentOverrideCreateEnumerator(CFASSFileDialogueTextContentOverrideRef override);

#endif /* CFASSFileDialogueTextContentOverride_h */
