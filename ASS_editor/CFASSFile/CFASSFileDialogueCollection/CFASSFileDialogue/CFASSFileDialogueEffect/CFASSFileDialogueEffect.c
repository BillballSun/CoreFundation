//
//  CFASSFileDialogueEffect.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

#include "CFASSFileDialogueEffect.h"
#include "CFASSFileDialogueEffect_Private.h"
#include "CFUseTool.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFException.h"

typedef enum CFASSFileDialogueEffectType
{
    CFASSFileDialogueEffectTypeKaraoke,
    CFASSFileDialogueEffectTypeScroll,
    CFASSFileDialogueEffectTypeBanner
} CFASSFileDialogueEffectType;

struct CFASSFileDialogueEffect {
    CFASSFileDialogueEffectType type;
    union
    {
        struct
        {
            CFASSFileDialogueEffectScrollDirection scrollDirection;
            int positionY1, positionY2;
            unsigned int delay;
            bool hasFadeAwayHeight;
            int fadeAwayHeight;
        }scroll;
        struct
        {
            unsigned int delay;
            bool usingDefaultDirection;
            bool leftToRight;
            bool hasFadeAwayWidth;
            int fadeAwayWidth;
        }banner;
    }data;
};

void CFASSFileDialogueEffectMakeChange(CFASSFileDialogueEffectRef effect, CFASSFileChangeRef change)
{
    if(effect == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueEffect %p MakeChange %p", effect, change);
}

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCopy(CFASSFileDialogueEffectRef effect)
{
    CFASSFileDialogueEffectRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueEffect))) != NULL)
    {
        *result = *effect;
        return result;
    }
    return NULL;
}

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateWithString(const wchar_t *beginPoint, const wchar_t *endPoint)
{
    // Karaoke
    // Scroll up;y1;y2;delay[;fadeawayheight]
    // Scroll down;y1;y2;delay[;fadeawayheight]
    // Banner;delay[;lefttoright;fadeawaywidth]
    
    bool checkMark = false;
    
    CFASSFileDialogueEffectType effectType = CFASSFileDialogueEffectTypeKaraoke;
    
    CFASSFileDialogueEffectScrollDirection scrollDirection = CFASSFileDialogueEffectScrollDirectionUp;
    int positionY1 = 0, positionY2 = 0;
    unsigned int delay = 0;
    bool hasFadeAwayHeight = false;
    int fadeAwayHeight = 0;
    
    bool usingDefaultDirection = true;
    int leftToRight_int = 0;
    bool leftToRight = false;
    bool hasFadeAwayWidth = false;
    int fadeAwayWidth = 0;
    
    if(CF_wchar_string_match_beginning(beginPoint, L"Karaoke"))
    {
        checkMark = true;
        effectType = CFASSFileDialogueEffectTypeKaraoke;
    }
    else if(swscanf(beginPoint, L"Scroll up;%d;%d;%u;%d", &positionY1, &positionY2, &delay, &fadeAwayHeight) == 4)
    {
        checkMark = true;
        effectType = CFASSFileDialogueEffectTypeScroll;
        scrollDirection = CFASSFileDialogueEffectScrollDirectionUp;
        hasFadeAwayHeight = true;
    }
    else if(swscanf(beginPoint, L"Scroll up;%d;%d;%u", &positionY1, &positionY2, &delay) == 3)
    {
        checkMark = true;
        effectType = CFASSFileDialogueEffectTypeScroll;
        scrollDirection = CFASSFileDialogueEffectScrollDirectionUp;
        hasFadeAwayHeight = false;
    }
    else if(swscanf(beginPoint, L"Scroll down;%d;%d;%u;%d", &positionY1, &positionY2, &delay, &fadeAwayHeight) == 4)
    {
        checkMark = true;
        effectType = CFASSFileDialogueEffectTypeScroll;
        scrollDirection = CFASSFileDialogueEffectScrollDirectionDown;
        hasFadeAwayHeight = true;
    }
    else if(swscanf(beginPoint, L"Scroll down;%d;%d;%u", &positionY1, &positionY2, &delay) == 3)
    {
        checkMark = true;
        effectType = CFASSFileDialogueEffectTypeScroll;
        scrollDirection = CFASSFileDialogueEffectScrollDirectionDown;
        hasFadeAwayHeight = false;
    }
    else if(swscanf(beginPoint, L"Banner;%u;%d;%d", &delay, &leftToRight_int, &fadeAwayWidth) == 3)
    {
        if(leftToRight_int == 0 || leftToRight_int == 1)
        {
            checkMark = true;
            effectType = CFASSFileDialogueEffectTypeBanner;
            leftToRight = leftToRight_int;
            usingDefaultDirection = false;
            hasFadeAwayWidth = true;
        }
    }
    else if(swscanf(beginPoint, L"Banner;%u", &delay) == 1)
    {
        checkMark = true;
        effectType = CFASSFileDialogueEffectTypeBanner;
        usingDefaultDirection = true;
        hasFadeAwayWidth = false;
    }
    if(checkMark)
    {
        CFASSFileDialogueEffectRef result;
        switch (effectType) {
            case CFASSFileDialogueEffectTypeKaraoke:
                result = CFASSFileDialogueEffectCreateKaraoke();
                break;
            case CFASSFileDialogueEffectTypeScroll:
                result = CFASSFileDialogueEffectCreateScroll(scrollDirection, positionY1, positionY2, delay, hasFadeAwayHeight, fadeAwayHeight);
                break;
            case CFASSFileDialogueEffectTypeBanner:
                result = CFASSFileDialogueEffectCreateBanner(delay, usingDefaultDirection, leftToRight, hasFadeAwayWidth, fadeAwayWidth);
                break;
        }
        return result;

    }
    return NULL;
}
int CFASSFileDialogueEffectStoreStringResult(CFASSFileDialogueEffectRef effect, wchar_t *targetPoint)
{
    int result = -1;
    if(targetPoint != NULL)
        switch (effect->type)
        {
        case CFASSFileDialogueEffectTypeKaraoke:
            result = swprintf(targetPoint, SIZE_MAX, L"Karaoke");
            break;
        case CFASSFileDialogueEffectTypeScroll:
            if(effect->data.scroll.hasFadeAwayHeight)
                result = swprintf(targetPoint, SIZE_MAX, L"Scroll %ls;%d;%d;%u;%d",
                                  effect->data.scroll.scrollDirection == CFASSFileDialogueEffectScrollDirectionUp?L"up":L"down",
                                  effect->data.scroll.positionY1, effect->data.scroll.positionY2,
                                  effect->data.scroll.delay,
                                  effect->data.scroll.fadeAwayHeight);
            else
                result = swprintf(targetPoint, SIZE_MAX, L"Scroll %ls;%d;%d;%u",
                                  effect->data.scroll.scrollDirection == CFASSFileDialogueEffectScrollDirectionUp?L"up":L"down",
                                  effect->data.scroll.positionY1, effect->data.scroll.positionY2,
                                  effect->data.scroll.delay);
            break;
        case CFASSFileDialogueEffectTypeBanner:
            if(!effect->data.banner.usingDefaultDirection && effect->data.banner.hasFadeAwayWidth)
                result = swprintf(targetPoint, SIZE_MAX, L"Banner;%u;%u;%d",
                                  effect->data.banner.delay,
                                  effect->data.banner.leftToRight?1u:0u,
                                  effect->data.banner.fadeAwayWidth);
            else if(!effect->data.banner.usingDefaultDirection && !effect->data.banner.hasFadeAwayWidth)
                result = swprintf(targetPoint, SIZE_MAX, L"Banner;%u;%u",
                                  effect->data.banner.delay,
                                  effect->data.banner.leftToRight?1u:0u);
            else if(effect->data.banner.usingDefaultDirection && effect->data.banner.hasFadeAwayWidth)
                result = swprintf(targetPoint, SIZE_MAX, L"Banner;%u;%d",
                                  effect->data.banner.delay,
                                  effect->data.banner.fadeAwayWidth);
            else if(effect->data.banner.usingDefaultDirection && !effect->data.banner.hasFadeAwayWidth)
                result = swprintf(targetPoint, SIZE_MAX, L"Banner;%u", effect->data.banner.delay);
            break;
        }
    else
    {
        FILE *fp = tmpfile();
        if(fp == NULL) return -1;
        switch (effect->type)
        {
            case CFASSFileDialogueEffectTypeKaraoke:
                result = fwprintf(fp, L"Karaoke");
                break;
            case CFASSFileDialogueEffectTypeScroll:
                if(effect->data.scroll.hasFadeAwayHeight)
                    result = fwprintf(fp, L"Scroll %ls;%d;%d;%u;%d",
                                    effect->data.scroll.scrollDirection == CFASSFileDialogueEffectScrollDirectionUp?L"up":L"down",
                                    effect->data.scroll.positionY1, effect->data.scroll.positionY2,
                                    effect->data.scroll.delay,
                                    effect->data.scroll.fadeAwayHeight);
                else
                    result = fwprintf(fp, L"Scroll %ls;%d;%d;%u",
                                    effect->data.scroll.scrollDirection == CFASSFileDialogueEffectScrollDirectionUp?L"up":L"down",
                                    effect->data.scroll.positionY1, effect->data.scroll.positionY2,
                                    effect->data.scroll.delay);
                break;
            case CFASSFileDialogueEffectTypeBanner:
                if(!effect->data.banner.usingDefaultDirection && effect->data.banner.hasFadeAwayWidth)
                    result = fwprintf(fp, L"Banner;%u;%u;%d",
                                      effect->data.banner.delay,
                                      effect->data.banner.leftToRight?1u:0u,
                                      effect->data.banner.fadeAwayWidth);
                else if(!effect->data.banner.usingDefaultDirection && !effect->data.banner.hasFadeAwayWidth)
                    result = fwprintf(fp, L"Banner;%u;%u",
                                      effect->data.banner.delay,
                                      effect->data.banner.leftToRight?1u:0u);
                else if(effect->data.banner.usingDefaultDirection && effect->data.banner.hasFadeAwayWidth)
                    result = fwprintf(fp, L"Banner;%u;%d",
                                      effect->data.banner.delay,
                                      effect->data.banner.fadeAwayWidth);
                else if(effect->data.banner.usingDefaultDirection && !effect->data.banner.hasFadeAwayWidth)
                    result = fwprintf(fp, L"Banner;%u", effect->data.banner.delay);
                break;
        }
        fclose(fp);
    }
    if(result<0) return -1;
    return result;
}

void CFASSFileDialogueEffectDestory(CFASSFileDialogueEffectRef effect)
{
    if(effect == NULL) return;
    free(effect);
}

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateKaraoke(void)
{
    CFASSFileDialogueEffectRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueEffect))) != NULL)
    {
        result->type = CFASSFileDialogueEffectTypeKaraoke;
        return result;
    }
    return NULL;
}

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateScroll(CFASSFileDialogueEffectScrollDirection scrollDirection,
                                                               int positionY1,
                                                               int positionY2,
                                                               unsigned int delay,
                                                               bool hasFadeAwayHeight,
                                                               int fadeAwayHeight)
{
    if(delay>100) return NULL;
    
    CFASSFileDialogueEffectRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueEffect))) != NULL)
    {
        result->type = CFASSFileDialogueEffectTypeScroll;
        result->data.scroll.scrollDirection = scrollDirection;
        result->data.scroll.positionY1 = positionY1;
        result->data.scroll.positionY2 = positionY2;
        result->data.scroll.delay = delay;
        result->data.scroll.hasFadeAwayHeight = hasFadeAwayHeight;
        result->data.scroll.fadeAwayHeight = fadeAwayHeight;
        return result;
    }
    return NULL;
}

CFASSFileDialogueEffectRef CFASSFileDialogueEffectCreateBanner(unsigned int delay,
                                                               bool usingDefaultDirection,
                                                               bool leftToRight,
                                                               bool hasFadeAwayWidth,
                                                               int fadeAwayWidth)
{
    if(delay>100) return NULL;
    
    CFASSFileDialogueEffectRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueEffect))) != NULL)
    {
        result->type = CFASSFileDialogueEffectTypeBanner;
        result->data.banner.delay = delay;
        result->data.banner.usingDefaultDirection = usingDefaultDirection;
        result->data.banner.leftToRight = leftToRight;
        result->data.banner.hasFadeAwayWidth = hasFadeAwayWidth;
        result->data.banner.fadeAwayWidth = fadeAwayWidth;
        return result;
    }
    return NULL;
}



















































