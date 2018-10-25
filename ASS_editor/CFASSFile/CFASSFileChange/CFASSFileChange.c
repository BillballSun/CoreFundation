//
//  CFASSFileChange.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/6.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>

#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFGeometry.h"
#include "CFException.h"
#include "CFUseTool.h"

static CFASSFileChangeRef CFASSFileChangeEmpty(void);
static bool CFASSFileChangeCombineInto(CFASSFileChangeRef dest, CFASSFileChangeRef source);

#pragma mark - Dispatch Rule

bool CFASSFileChangeShouldDispatchToScriptInfo(CFASSFileChangeRef change)
{
    if(change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileChange NULL ShouldDispatchToScriptInfo");
    if(change->type & CFASSFileChangeTypeResolution)
        return true;
    return false;
}

bool CFASSFileChangeShouldDispatchToStyleCollection(CFASSFileChangeRef change)
{
    if(change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileChange NULL ShouldDispatchToStyleCollection");
    if(change->type & CFASSFileChangeTypeFontSize)
        return true;
    if(change->type & CFASSFileChangeTypeFontName)
        return true;
    if(change->type & CFASSFileChangeTypeOutlineColor)
        return true;
    if(change->type & CFASSFileChangeTypePrimaryColor)
        return true;
    if(change->type & CFASSFileChangeTypeBlod)
        return true;
    if(change->type & CFASSFileChangeTypeBorderStyle)
        return true;
    if(change->type & CFASSFileChangeTypeOutline)
        return true;
    if(change->type & CFASSFileChangeTypeShadow)
        return true;
    return false;
}

bool CFASSFileChangeShouldDispatchToDialogueCollection(CFASSFileChangeRef change)
{
    if(change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileChange NULL ShouldDispatchToStyleCollection");
    if((change->type & CFASSFileChangeTypeFontSize) && change->fontSize.affectOverride)
        return true;
    if((change->type & CFASSFileChangeTypeFontName) && change->fontName.affectOverride)
        return true;
    if(change->type & CFASSFileChangeTypeTimeOffset)
        return true;
    if(change->type & CFASSFileChangeTypeDiscardAllOverride)
        return true;
    return false;
}

bool CFASSFileChangeShouldDispatchToDialogueTextContentOverride(CFASSFileChangeRef change)
{
    if(change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileChange NULL ShouldDispatchToDialogueTextContentOverride");
    if((change->type & CFASSFileChangeTypeFontSize) && change->fontSize.affectOverride)
        return true;
    if((change->type & CFASSFileChangeTypeFontName) && change->fontName.affectOverride)
        return true;
    return false;
}

#pragma mark - Change Creation

CFASSFileChangeRef CFASSFileChangeBorderStyle(int borderStyle)
{
    if(borderStyle!=1 && borderStyle!=3) return NULL;
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeBorderStyle;
        result->borderStyle.borderStyle = borderStyle;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeOutlineWidth(unsigned int outlinePixelWidth, bool forceBorderStyle)
{
    if(outlinePixelWidth>4) outlinePixelWidth = 4;
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeOutline;
        result->outline.forceBorderStyle = forceBorderStyle;
        result->outline.pixel = outlinePixelWidth;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeShadowWidth(unsigned int shadowPixelWidth, bool forceBorderStyle)
{
    if(shadowPixelWidth>4) shadowPixelWidth = 4;
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeShadow;
        result->shadow.forceBorderStyle = forceBorderStyle;
        result->shadow.pixel = shadowPixelWidth;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeTimeOffset(long hundredths)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeTimeOffset;
        result->timeOffset.hundredths = hundredths;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangePrimaryColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypePrimaryColor;
        result->primaryColor.red = red;
        result->primaryColor.green = green;
        result->primaryColor.blue = blue;
        result->primaryColor.alpha = alpha;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeOutlineColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeOutlineColor;
        result->outlineColor.red = red;
        result->outlineColor.green = green;
        result->outlineColor.blue = blue;
        result->outlineColor.alpha = alpha;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeFontSize(bool usingPercentage, double percentage, unsigned int newSize)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeFontSize;
        result->fontSize.byPercentage = usingPercentage;
        result->fontSize.fontSize = newSize;
        result->fontSize.percentage = percentage;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeFontName(wchar_t *fontName, bool affectOverride)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        if((result->fontName.fontName = CF_Dump_wchar_string(fontName)) != NULL)
        {
            result->type = CFASSFileChangeTypeFontName;
            result->fontName.affectOverride = affectOverride;
            return result;
        }
        CFASSFileChangeDestory(result);
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeResolution(CFUSize newSize)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeResolution;
        result->resolution.newSize = newSize;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeDiscardAllOverride(void)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeDiscardAllOverride;
        return result;
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeBlod(bool blod)
{
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        result->type = CFASSFileChangeTypeBlod;
        result->blod.isBlod = blod;
        return result;
    }
    return NULL;
}

static CFASSFileChangeRef CFASSFileChangeEmpty(void)
{
    CFASSFileChangeRef result;
    if((result = malloc(sizeof(struct CFASSFileChange))) != NULL)
    {
        result->type = CFASSFileChangeTypeNone;
        result->fontName.fontName = NULL;
        return result;
    }
    return NULL;
}

void CFASSFileChangeDestory(CFASSFileChangeRef change)
{
    if(change == NULL) return;
    if(change->type & CFASSFileChangeTypeFontName)
        free(change->fontName.fontName);
    free(change);
}

CFASSFileChangeRef CFASSFileMultiChangeCombineTermiateWithNULL(bool transferOwnershipIfCombinationSuccess, CFASSFileChangeRef change1, ...)
{
    if(change1 == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileMultiChangeCombineTermiateWithNULL has nothing");
    va_list ap;
    va_start(ap, change1);
    CFASSFileChangeType allType = change1->type;
    CFASSFileChangeRef eachChange;
    while((eachChange = va_arg(ap, CFASSFileChangeRef)) != NULL)
    {
        if(allType & eachChange->type)
        {
            va_end(ap);
            return NULL;
        }
        allType |= eachChange->type;
    }
    va_end(ap);
    
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()) != NULL)
    {
        if(CFASSFileChangeCombineInto(result, change1))
        {
            va_start(ap, change1);
            while((eachChange = va_arg(ap, CFASSFileChangeRef)) != NULL)
            {
                if(!CFASSFileChangeCombineInto(result, eachChange))
                    break;
            }
            va_end(ap);
            if(eachChange == NULL)
            {
                if(transferOwnershipIfCombinationSuccess)
                {
                    CFASSFileChangeDestory(change1);
                    va_start(ap, change1);
                    while((eachChange = va_arg(ap, CFASSFileChangeRef)) != NULL)
                        CFASSFileChangeDestory(eachChange);
                    va_end(ap);
                }
                return result;
            }
        }
        CFASSFileChangeDestory(result);
    }
    return NULL;
}

CFASSFileChangeRef CFASSFileChangeCombine(CFASSFileChangeRef change1, CFASSFileChangeRef change2, bool transferOwnershipIfCombinationSuccess)
{
    if(change1 == NULL || change2 == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileCombine %p and %p", change1, change2);
    if(change1->type & change2->type) return NULL;
    CFASSFileChangeRef result;
    if((result = CFASSFileChangeEmpty()))
    {
        if(CFASSFileChangeCombineInto(result, change1))
        {
            if(CFASSFileChangeCombineInto(result, change2))
            {
                if(transferOwnershipIfCombinationSuccess)
                {
                    CFASSFileChangeDestory(change1);
                    CFASSFileChangeDestory(change2);
                }
                return result;
            }
        }
        CFASSFileChangeDestory(result);
    }
    return NULL;
}

static bool CFASSFileChangeCombineInto(CFASSFileChangeRef dest, CFASSFileChangeRef source)
{
    if(dest->type & source->type) return false;
    if(source->type & CFASSFileChangeTypeFontName)
    {
        dest->fontName.fontName = CF_Dump_wchar_string(source->fontName.fontName);
        if(dest->fontName.fontName == NULL)
            return false;
        dest->type |= CFASSFileChangeTypeFontName;      // exist latter
        dest->fontName.affectOverride = dest->fontName.affectOverride;
    }
    if(source->type & CFASSFileChangeTypeResolution)
    {
        dest->type |= CFASSFileChangeTypeResolution;
        dest->resolution = source->resolution;
    }
    if(source->type & CFASSFileChangeTypeFontSize)
    {
        dest->type |= CFASSFileChangeTypeFontSize;
        dest->fontSize = source->fontSize;
    }
    if(source->type & CFASSFileChangeTypePrimaryColor)
    {
        dest->type |= CFASSFileChangeTypePrimaryColor;
        dest->primaryColor = source->primaryColor;
    }
    if(source->type & CFASSFileChangeTypeOutlineColor)
    {
        dest->type |= CFASSFileChangeTypeOutlineColor;
        dest->outlineColor = source->outlineColor;
    }
    if(source->type & CFASSFileChangeTypeTimeOffset)
    {
        dest->type |= CFASSFileChangeTypeTimeOffset;
        dest->timeOffset = source->timeOffset;
    }
    if(source->type & CFASSFileChangeTypeDiscardAllOverride)
    {
        dest->type |= CFASSFileChangeTypeDiscardAllOverride;
    }
    if(source->type & CFASSFileChangeTypeBlod)
    {
        dest->type |= CFASSFileChangeTypeBlod;
        dest->blod = source->blod;
    }
    if(source->type & CFASSFileChangeTypeBorderStyle)
    {
        dest->type |= CFASSFileChangeTypeBorderStyle;
        dest->borderStyle = source->borderStyle;
    }
    if(source->type & CFASSFileChangeTypeOutline)
    {
        dest->type |= CFASSFileChangeTypeOutline;
        dest->outline = source->outline;
    }
    if(source->type & CFASSFileChangeTypeShadow)
    {
        dest->type |= CFASSFileChangeTypeShadow;
        dest->shadow = source->shadow;
    }
    return true;
}
