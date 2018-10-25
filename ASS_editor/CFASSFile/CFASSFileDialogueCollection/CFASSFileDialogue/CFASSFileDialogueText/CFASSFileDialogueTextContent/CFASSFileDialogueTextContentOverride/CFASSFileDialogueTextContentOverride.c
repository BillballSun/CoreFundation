//
//  CFASSFileDialogueTextContentOverride.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <stddef.h>

#include "CFPointerArray.h"
#include "CFASSFileDialogueTextContentOverride.h"
#include "CFASSFileDialogueTextContentOverride_Private.h"
#include "CFASSFileDialogueTextContentOverrideContent.h"
#include "CFASSFileDialogueTextContentOverrideContent_Private.h"
#include "CFUseTool.h"
#include "CFException.h"
#include "CFEnumerator.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"

struct CFASSFileDialogueTextContentOverride
{
    CFPointerArrayRef contentArray;
};

void CFASSFileDialogueTextContentOverrideMakeChange(CFASSFileDialogueTextContentOverrideRef override, CFASSFileChangeRef change)
{
    if(override == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride %p MakeChange %p", override, change);
    CFEnumeratorRef enumerator = CFASSFileDialogueTextContentOverrideCreateEnumerator(override);
    CFASSFileDialogueTextContentOverrideContentRef eachContent;
    while((eachContent = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileDialogueTextContentOverrideContentMakeChange(eachContent, change);
    CFEnumeratorDestory(enumerator);
}

CFEnumeratorRef CFASSFileDialogueTextContentOverrideCreateEnumerator(CFASSFileDialogueTextContentOverrideRef override)
{
    if(override == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(override->contentArray);
}

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithContent(CFASSFileDialogueTextContentOverrideContentRef content)
{
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverrideCreateWithContent NULL");
        return NULL;
    }
    CFASSFileDialogueTextContentOverrideRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverride))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            CFPointerArrayAddPointer(result->contentArray, content, false);
            return result;
        }
        free(result);
    }
    return NULL;
}

void CFASSFileDialogueTextContentOverrideAddContent(CFASSFileDialogueTextContentOverrideRef override, CFASSFileDialogueTextContentOverrideContentRef content)
{
    if(override == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride NULL AddContent");
        return;
    }
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride %X AddContent NULL", (unsigned int)override);
        return;
    }
    CFPointerArrayAddPointer(override->contentArray, content, false);
}

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCopy(CFASSFileDialogueTextContentOverrideRef override)
{
    CFASSFileDialogueTextContentOverrideRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverride))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayLength = CFPointerArrayGetLength(override->contentArray);
            bool copyCheck = true;
            CFASSFileDialogueTextContentOverrideContentRef eachOverrideContent;
            for(size_t index = 0; index < arrayLength && copyCheck; index++)
            {
                eachOverrideContent = CFASSFileDialogueTextContentOverrideContentCopy
                ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index));
                if(eachOverrideContent == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->contentArray, eachOverrideContent, false);
            }
            if(copyCheck)
                return result;
            arrayLength = CFPointerArrayGetLength(result->contentArray);
            for(size_t index = 0; index < arrayLength; index++)
                CFASSFileDialogueTextContentOverrideContentDestory
                ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(result->contentArray, index));
        }
        free(result);
    }
    return NULL;
}

void CFASSFileDialogueTextContentOverrideDestory(CFASSFileDialogueTextContentOverrideRef override)
{
    if(override == NULL) return;
    size_t arrayLength = CFPointerArrayGetLength(override->contentArray);
    for(size_t index = 0; index<arrayLength; index++)
        CFASSFileDialogueTextContentOverrideContentDestory
        ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index));
    CFPointerArrayDestory(override->contentArray);
    free(override);
}

int CFASSFileDialogueTextContentOverrideStoreStringResult(CFASSFileDialogueTextContentOverrideRef override, wchar_t * targetPoint)
{
    if(override == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverride NULL StoreStringResult");
        return -1;
    }
    int result = 0, temp;
    if(targetPoint == NULL)
    {
        size_t arrayLength = CFPointerArrayGetLength(override->contentArray);
        for(size_t index = 0; index<arrayLength; index++)
        {
            temp = CFASSFileDialogueTextContentOverrideContentStoreStringResult
            ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index), NULL);
            if(temp < 0) return -1;
            else result += temp;
        }
        return result + 2;  /* braces */
    }
    else
    {
        *targetPoint++ = L'{';
        size_t arrayLength = CFPointerArrayGetLength(override->contentArray);
        for(size_t index = 0; index<arrayLength; index++)
        {
            temp = CFASSFileDialogueTextContentOverrideContentStoreStringResult
            ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(override->contentArray, index), targetPoint);
            if(temp < 0)
                return -1;
            else
            {
                result += temp;
                targetPoint += temp;
            }
        }
        targetPoint[0] = L'}';
        targetPoint[1] = L'\0';
        return result + 2;  /* braces */
    }
}

CFASSFileDialogueTextContentOverrideRef CFASSFileDialogueTextContentOverrideCreateWithString(const wchar_t *string, bool isIncludeBraces)
{
    CFASSFileDialogueTextContentOverrideRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverride))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            const wchar_t *beginPoint = string;
            const wchar_t *endPoint = NULL;
            bool checkMark = false;
            if(isIncludeBraces)
            {
                beginPoint++;
                if(*string == L'{' && *beginPoint=='\\')
                {
                    endPoint = beginPoint;
                    while(*endPoint!=L'}' && *endPoint!=L'\0') endPoint++;
                    if(*endPoint == L'}')
                    {
                        endPoint--;
                        checkMark = true;
                    }
                }
            }
            else if(*beginPoint=='\\')
            {
                endPoint = string + wcslen(string) - 1;
                checkMark = true;
            }
            if(checkMark)
            {
                const wchar_t *tokenBegin = beginPoint, *tokenEnd;
                bool formatCheck = true;
                do
                {
                    tokenEnd = tokenBegin+1;
                    if(tokenEnd>endPoint)
                    {
                        formatCheck = false;
                        break;
                    }
                    while(tokenEnd<endPoint && *tokenEnd!=L'\\' && formatCheck)
                    {
                        if(*tokenEnd==L'(')
                        {
                            const wchar_t *matchBrace = CF_match_next_braces(tokenEnd, endPoint, true);
                            if(matchBrace!=NULL)
                                tokenEnd = matchBrace;
                            else
                                formatCheck = false;
                        }
                        tokenEnd++;
                    }
                    if(formatCheck)
                    {
                        if(*tokenEnd==L'\\')
                            tokenEnd--;
                        CFASSFileDialogueTextContentOverrideContentRef content;
                        if((content = CFASSFileDialogueTextContentOverrideContentCreateWithString(tokenBegin, tokenEnd))!=NULL)
                            CFPointerArrayAddPointer(result->contentArray, content, false);
                        else
                            formatCheck = false;
                        tokenBegin = tokenEnd + 1;
                    }
                }while(formatCheck && tokenEnd<endPoint);
                if(formatCheck)
                    return result;
            }
            size_t arrayLength = CFPointerArrayGetLength(result->contentArray);
            for(size_t index = 0; index<arrayLength; index++)
                CFASSFileDialogueTextContentOverrideContentDestory
                ((CFASSFileDialogueTextContentOverrideContentRef)CFPointerArrayGetPointerAtIndex(result->contentArray, index));
            CFPointerArrayDestory(result->contentArray);
        }
        free(result);
    }
    return NULL;
}
