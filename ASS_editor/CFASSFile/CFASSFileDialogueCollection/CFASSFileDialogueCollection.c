//
//  CFASSFileDialogueCollection.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <wchar.h>
#include <stdlib.h>

#include "CFASSFile.h"
#include "CFASSFile_Private.h"
#include "CFASSFileDialogueCollection.h"
#include "CFASSFileDialogueCollection_Private.h"
#include "CFASSFileDialogue.h"
#include "CFASSFileDialogue_Private.h"
#include "CFPointerArray.h"
#include "CFEnumerator.h"
#include "CFException.h"
#include "CFASSFileControl.h"
#include "CFASSFileControl_Private.h"

struct CFASSFileDialogueCollection
{
    CFPointerArrayRef collectionArray;
    CFASSFileRef registeredFile;
};

void CFASSFileDialogueCollectionMakeChange(CFASSFileDialogueCollectionRef dialogueCollection, CFASSFileChangeRef change)
{
    if(dialogueCollection == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueCollection %p MakeChange %p", dialogueCollection, change);
    CFEnumeratorRef enumerator = CFASSFileDialogueCollectionCreateEnumerator(dialogueCollection);
    CFASSFileDialogueRef eachDialogue;
    while ((eachDialogue = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileDialogueMakeChange(eachDialogue, change);
    CFEnumeratorDestory(enumerator);
}

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCopy(CFASSFileDialogueCollectionRef dialogueCollection)
{
    CFASSFileDialogueCollectionRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueCollection))) != NULL)
    {
        result->registeredFile = NULL;
        if((result->collectionArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayLength = CFPointerArrayGetLength(dialogueCollection->collectionArray);
            bool copyCheck = true;
            CFASSFileDialogueRef eachDialogue;
            for(size_t index = 0; index<arrayLength && copyCheck; index++)
            {
                eachDialogue = CFASSFileDialogueCopy
                ((CFASSFileDialogueRef)CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index));
                if(eachDialogue == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->collectionArray, eachDialogue, false);
            }
            if(copyCheck)
                return result;
            arrayLength = CFPointerArrayGetLength(result->collectionArray);
            for(size_t index = 0; index<arrayLength; index++)
                CFASSFileDialogueDestory
                ((CFASSFileDialogueRef)CFPointerArrayGetPointerAtIndex(result->collectionArray, index));
            CFPointerArrayDestory(result->collectionArray);
        }
        free(result);
    }
    return NULL;
}

CFEnumeratorRef CFASSFileDialogueCollectionCreateEnumerator(CFASSFileDialogueCollectionRef dialogueCollection)
{
    if(dialogueCollection == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueCollection NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(dialogueCollection->collectionArray);
}

void CFASSFileDialogueCollectionDestory(CFASSFileDialogueCollectionRef dialogueCollection)
{
    if(dialogueCollection == NULL) return;
    size_t arrayLength = CFPointerArrayGetLength(dialogueCollection->collectionArray);
    for(size_t index = 0; index<arrayLength; index++)
        CFASSFileDialogueDestory(CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index));
    CFPointerArrayDestory(dialogueCollection->collectionArray);
    free(dialogueCollection);
}

int CFASSFileDialogueCollectionRegisterAssociationwithFile(CFASSFileDialogueCollectionRef collection, CFASSFileRef assFile)
{
    if(collection->registeredFile == NULL)
    {
        collection->registeredFile = assFile;
        return 0;
    }
    else
        return -1;
}

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCreateWithUnicodeFileContent(const wchar_t *content)
{
    wchar_t *searchPoint;
    if((searchPoint = wcsstr(content, L"\n[Events]")) != NULL)
    {
        searchPoint = wcsstr(searchPoint, L"\nDialogue:");
        if(searchPoint == NULL)
            return NULL;
        CFASSFileDialogueCollectionRef result;
        if((result = malloc(sizeof(struct CFASSFileDialogueCollection))) != NULL)
        {
            result->registeredFile = NULL;
            if((result->collectionArray = CFPointerArrayCreateEmpty())!=NULL)
            {
                size_t skipLength = wcslen(L"\nDialogue:");
                CFASSFileDialogueRef eachDialogue;
                bool formatCheck = true;
                while(formatCheck && (searchPoint = wcsstr(searchPoint, L"\nDialogue:")) != NULL)
                {
                    eachDialogue = CFASSFileDialogueCreateWithString(searchPoint+1);
                    if(eachDialogue == NULL)
                    {
                        CFASSFileControlErrorHandling errorHandle = CFASSFileControlGetErrorHandling();
                        if(!(errorHandle & CFASSFileControlErrorHandlingIgnore))
                            formatCheck = false;
                        else
                            searchPoint += skipLength;
                        if(errorHandle & CFASSFileControlErrorHandlingOutput)
                            CFASSFileControlErrorOutput(content, searchPoint+1);
                    }
                    else
                    {
                        CFPointerArrayAddPointer(result->collectionArray, eachDialogue, false);
                        searchPoint += skipLength;
                    }
                }
                if(formatCheck)
                    return result;
                size_t arrayLength = CFPointerArrayGetLength(result->collectionArray);
                for(size_t index = 0; index<arrayLength; index++)
                    CFASSFileDialogueDestory(CFPointerArrayGetPointerAtIndex(result->collectionArray, index));
                CFPointerArrayDestory(result->collectionArray);
            }
            free(result);
        }
    }
    return NULL;
}

wchar_t *CFASSFileDialogueCollectionAllocateFileContent(CFASSFileDialogueCollectionRef dialogueCollection)
{
    size_t stringLength = 0,
    prefixLength = wcslen(L"[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n");
    
    stringLength += prefixLength;
    
    int temp;
    size_t arrayLength = CFPointerArrayGetLength(dialogueCollection->collectionArray);
    for(size_t index = 0; index<arrayLength; index++)
    {
        temp = CFASSFileDialogueStoreStringResult(CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index), NULL);
        if(temp == -1) return NULL;
        else stringLength += temp;
    }
    
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t)*(stringLength+1)))!=NULL)
    {
        swprintf(result, SIZE_MAX, L"[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n");
        wchar_t *currentPoint = result + prefixLength;
        bool formatCheck = true;
        for(size_t index = 0; index<arrayLength && formatCheck; index++)
        {
            temp = CFASSFileDialogueStoreStringResult(CFPointerArrayGetPointerAtIndex(dialogueCollection->collectionArray, index), currentPoint);
            if(temp<0)
                formatCheck = false;
            else
                currentPoint += temp;
        }
        if(formatCheck)
            return result;
        free(result);
    }
    return NULL;
}
