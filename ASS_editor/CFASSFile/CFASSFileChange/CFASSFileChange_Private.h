//
//  CFASSFileChange_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/6.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileChange_Private_h
#define CFASSFileChange_Private_h

#include <stdbool.h>

#include "CFASSFileChange.h"

#pragma mark - Internal Structure

typedef enum {
    CFASSFileChangeTypeNone = 0,
    CFASSFileChangeTypeResolution = 1 << 0,
    CFASSFileChangeTypeFontSize = 1 << 1,
    CFASSFileChangeTypeFontName = 1 << 2,
    CFASSFileChangeTypePrimaryColor = 1 << 3,
    CFASSFileChangeTypeOutlineColor = 1 << 4,
    CFASSFileChangeTypeTimeOffset = 1 << 5,
    CFASSFileChangeTypeDiscardAllOverride = 1 << 6,
    CFASSFileChangeTypeBlod = 1 << 7,
    CFASSFileChangeTypeBorderStyle = 1 << 8,
    CFASSFileChangeTypeOutline = 1 << 9,
    CFASSFileChangeTypeShadow = 1 << 10,
} CFASSFileChangeType;

struct CFASSFileChange
{
    CFASSFileChangeType type;
    struct
    {
        CFUSize newSize;
    }resolution;
    struct
    {
        bool byPercentage;
        bool affectOverride;
        double percentage;
        unsigned int fontSize;
    } fontSize;
    struct
    {
        wchar_t *fontName;
        bool affectOverride;
    }fontName;
    struct
    {
        unsigned char red, green, blue, alpha;
    }primaryColor;
    struct
    {
        unsigned char red, green, blue, alpha;
    }outlineColor;
    struct
    {
        long hundredths;
    }timeOffset;
    struct
    {
        bool isBlod;
    }blod;
    struct
    {
        int borderStyle;
    }borderStyle;
    struct
    {
        unsigned int pixel;
        bool forceBorderStyle;
    }outline;
    struct
    {
        unsigned int pixel;
        bool forceBorderStyle;
    }shadow;
};

#pragma mark - Dispatch Rule

bool CFASSFileChangeShouldDispatchToScriptInfo(CFASSFileChangeRef change);

bool CFASSFileChangeShouldDispatchToStyleCollection(CFASSFileChangeRef change);

bool CFASSFileChangeShouldDispatchToDialogueCollection(CFASSFileChangeRef change);

bool CFASSFileChangeShouldDispatchToDialogueTextContentOverride(CFASSFileChangeRef change);

#endif /* CFASSFileChange_Private_h */
