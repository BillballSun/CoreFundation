//
//  CFASSFileStyleCollection.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "CFASSFile.h"
#include "CFASSFile_Private.h"
#include "CFASSFileStyleCollection.h"
#include "CFASSFileStyleCollection_Private.h"
#include "CFPointerArray.h"
#include "CFUseTool.h"
#include "CFASSFileStyle.h"
#include "CFASSFileStyle_Private.h"
#include "CFUnicodeStringArray.h"
#include "CFException.h"
#include "CFEnumerator.h"
#include "CFASSFileControl.h"
#include "CFASSFileControl_Private.h"

struct CFASSFileStyleCollection
{
    CFPointerArrayRef styleCollection;
    CFASSFileRef registeredFile;            // don't have ownership
};

static wchar_t const * const CFASSFileStyleCollectionDiscription = L"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n";

CFEnumeratorRef CFASSFileStyleCollectionCreateEnumerator(CFASSFileStyleCollectionRef styleCollection)
{
    if(styleCollection == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileStyleCollection NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(styleCollection->styleCollection);
}

void CFASSFileStyleCollectionMakeChange(CFASSFileStyleCollectionRef styleCollection, CFASSFileChangeRef change)
{
    if(styleCollection == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileStyleCollection %p MakeChange %p", styleCollection, change);
    CFEnumeratorRef enumerator = CFASSFileStyleCollectionCreateEnumerator(styleCollection);
    CFASSFileStyleRef eachStyle;
    while ((eachStyle = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileStyleMakeChange(eachStyle, change);
    CFEnumeratorDestory(enumerator);
}

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCopy(CFASSFileStyleCollectionRef styleCollection)
{
    CFASSFileStyleCollectionRef result;
    if((result = malloc(sizeof(struct CFASSFileStyleCollection))) != NULL)
    {
        if((result->styleCollection = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayLength = CFPointerArrayGetLength(styleCollection->styleCollection);
            CFASSFileStyleRef eachStyle;
            bool copyCheck = true;
            for(size_t index = 0; index<arrayLength && copyCheck; index++)
            {
                eachStyle = CFASSFileStyleCopy((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(styleCollection->styleCollection, index));
                if(eachStyle == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->styleCollection, eachStyle, false);
            }
            if(copyCheck)
            {
                result->registeredFile = NULL;
                return result;
            }
            arrayLength = CFPointerArrayGetLength(result->styleCollection);
            for(size_t index = 0; index<arrayLength; index++)
                CFASSFileStyleDestory((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(result->styleCollection, index));
            CFPointerArrayDestory(result->styleCollection);
        }
        free(result);
    }
    return NULL;
}

wchar_t *CFASSFileStyleCollectionAllocateFileContent(CFASSFileStyleCollectionRef styleCollection)
{
    size_t styleAmount = CFPointerArrayGetLength(styleCollection->styleCollection);
    CFUnicodeStringArrayRef stringArray;
    if((stringArray = CFUnicodeStringArrayCreateEmpty()) != NULL)
    {
        const wchar_t *eachStyle;
        bool isAllocatingSuccess = true;
        for(size_t count = 1; isAllocatingSuccess && count<=styleAmount; count++)
        {
            if((eachStyle =
                CFASSFileStyleAllocateFileContent
                ((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(styleCollection->styleCollection,
                                                                    count-1))) != NULL)
                CFUnicodeStringArrayAddString(stringArray, eachStyle, true);
            else
                isAllocatingSuccess = false;
        }
        if(isAllocatingSuccess)
        {
            size_t styleCollectionStringLength = wcslen(L"[V4+ Styles]\n") + wcslen(CFASSFileStyleCollectionDiscription);
            for(size_t count = 1; count<=styleAmount; count++)
                styleCollectionStringLength += wcslen(CFUnicodeStringArrayGetStringAtIndex(stringArray, count-1));
            
            wchar_t *result;
            if((result = malloc(sizeof(wchar_t)*(styleCollectionStringLength+1))) != NULL)
            {
                wchar_t *beginPoint = result;
                size_t eachLength;
                
                eachLength = wcslen(L"[V4+ Styles]\n");
                wmemcpy(beginPoint, L"[V4+ Styles]\n", eachLength);
                beginPoint += eachLength;
                
                eachLength = wcslen(CFASSFileStyleCollectionDiscription);
                wmemcpy(beginPoint, CFASSFileStyleCollectionDiscription, eachLength);
                beginPoint += eachLength;
                
                for(size_t count = 1; count<=styleAmount; count++)
                {
                    eachStyle = CFUnicodeStringArrayGetStringAtIndex(stringArray, count-1);
                    eachLength = wcslen(eachStyle);
                    wmemcpy(beginPoint, eachStyle, eachLength);
                    beginPoint += eachLength;
                }
                *beginPoint = L'\0';
                
                CFUnicodeStringArrayDestory(stringArray);
                return result;
            }
        }
        CFUnicodeStringArrayDestory(stringArray);
    }
    return NULL;
}

void CFASSFileStyleCollectionDestory(CFASSFileStyleCollectionRef styleCollection)
{
    if(styleCollection == NULL) return;
    size_t arrayLength = CFPointerArrayGetLength(styleCollection->styleCollection);
    for(size_t index = 0; index<arrayLength; index++)
        CFASSFileStyleDestory((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(styleCollection->styleCollection, index));
    CFPointerArrayDestory(styleCollection->styleCollection);
    free(styleCollection);
}

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCreateWithUnicodeFileContent(const wchar_t *content)
{
    CFASSFileStyleCollectionRef result;
    if((result = malloc(sizeof(struct CFASSFileStyleCollection))) != NULL)
    {
        if((result->styleCollection = CFPointerArrayCreateEmpty()) != NULL)
        {
            const wchar_t *beginPoint, *endPoint;
            if((beginPoint = wcsstr(content, L"\n[V4+ Styles]")) != NULL &&
               (endPoint = wcsstr(beginPoint, L"\n[Events]\n")) != NULL)
            {
                bool isFormatCorrect = true;
                beginPoint+=wcslen(L"\n[V4+ Styles]");
                
                CFASSFileStyleRef eachStyle;
                while(isFormatCorrect && (beginPoint = CF_wcsstr_with_end_point(beginPoint, L"\nStyle:", endPoint)) != NULL)
                {
                    beginPoint++;
                    if((eachStyle = CFASSFileStyleCreateWithString(beginPoint)) == NULL)
                    {
                        CFASSFileControlErrorHandling errorHandle = CFASSFileControlGetErrorHandling();
                        if(!(errorHandle & CFASSFileControlErrorHandlingIgnore))
                            isFormatCorrect = false;
                        if(errorHandle & CFASSFileControlErrorHandlingOutput)
                            CFASSFileControlErrorOutput(content, beginPoint);
                    }
                    else
                        CFPointerArrayAddPointer(result->styleCollection, eachStyle, false);
                }
                
                if(isFormatCorrect)
                {
                    result->registeredFile = NULL;
                    return result;
                }
            }
            size_t arrayLength = CFPointerArrayGetLength(result->styleCollection);
            for(size_t index = 0; index<arrayLength; index++)
                CFASSFileStyleDestory((CFASSFileStyleRef)CFPointerArrayGetPointerAtIndex(result->styleCollection, index));
            CFPointerArrayDestory(result->styleCollection);
        }
        free(result);
    }
    return NULL;
}

int CFASSFileStyleCollectionRegisterAssociationwithFile(CFASSFileStyleCollectionRef collection, CFASSFileRef assFile)
{
    if(collection->registeredFile == NULL)
    {
        collection->registeredFile = assFile;
        return 0;
    }
    else
        return -1;
}
