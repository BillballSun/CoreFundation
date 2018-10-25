//
//  CFASSFileDialogueTextContent.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContent_h
#define CFASSFileDialogueTextContent_h

#include "CFASSFileDialogueTextContentOverride.h"
#include "CFASSFileChange.h"

typedef enum CFASSFileDialogueTextContentType {
    CFASSFileDialogueTextContentTypeText,
    CFASSFileDialogueTextContentTypeOverride
} CFASSFileDialogueTextContentType;

typedef struct CFASSFileDialogueTextContent *CFASSFileDialogueTextContentRef;

#pragma mark - Create/Copy/Destory

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithText(wchar_t *text, bool transferAllocationOwnership);

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithOverride(CFASSFileDialogueTextContentOverrideRef override, bool transferOwnership);

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCopy(CFASSFileDialogueTextContentRef textContent);

void CFASSFileDialogueTextContentDestory(CFASSFileDialogueTextContentRef textContent);

CFASSFileDialogueTextContentType CFASSFileDialogueTextContentGetType(CFASSFileDialogueTextContentRef textContent);

#pragma mark - Receive Change

void CFASSFileDialogueTextContentMakeChange(CFASSFileDialogueTextContentRef textContent, CFASSFileChangeRef change);

#endif /* CFASSFileDialogueTextContent_h */
