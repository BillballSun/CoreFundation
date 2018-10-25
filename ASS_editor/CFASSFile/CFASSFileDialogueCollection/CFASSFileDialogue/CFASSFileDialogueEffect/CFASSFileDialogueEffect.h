//
//  CFASSFileDialogueEffect.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueEffect_h
#define CFASSFileDialogueEffect_h

#include <stdbool.h>

#include "CFASSFileChange.h"

typedef struct CFASSFileDialogueEffect *CFASSFileDialogueEffectRef;

typedef enum CFASSFileDialogueEffectScrollDirection {
    CFASSFileDialogueEffectScrollDirectionUp,
    CFASSFileDialogueEffectScrollDirectionDown
} CFASSFileDialogueEffectScrollDirection;

#pragma mark - Create/Copy/Destory

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateKaraoke(void);

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateScroll(CFASSFileDialogueEffectScrollDirection type,
                                                               int positionY1,
                                                               int positionY2,
                                                               unsigned int delay,
                                                               bool hasFadeAwayHeight,
                                                               int fadeAwayHeight);

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateBanner(unsigned int delay,
                                                               bool usingDefaultDirection,
                                                               bool leftToRight,
                                                               bool hasFadeAwayWidth,
                                                               int fadeAwayWidth);

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCopy(CFASSFileDialogueEffectRef effect);

void CFASSFileDialogueEffectDestory(CFASSFileDialogueEffectRef effect);

#pragma mark - Receive Change

void CFASSFileDialogueEffectMakeChange(CFASSFileDialogueEffectRef effect, CFASSFileChangeRef change);

#endif /* CFASSFileDialogueEffect_h */
