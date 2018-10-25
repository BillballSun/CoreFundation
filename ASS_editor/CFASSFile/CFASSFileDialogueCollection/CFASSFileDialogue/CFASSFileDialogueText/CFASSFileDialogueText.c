//
//  CFASSFileDialogueText.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>

#include "CFASSFileDialogueText.h"
#include "CFASSFileDialogueText_Private.h"
#include "CFASSFileDialogueTextContent.h"
#include "CFASSFileDialogueTextContent_Private.h"
#include "CFPointerArray.h"
#include "CFException.h"
#include "CFEnumerator.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"

struct CFASSFileDialogueText
{
    CFPointerArrayRef contentArray;     // Can't be empty, must at least have CFASSFileDialogueTextContentCreateEmptyString();
};

void CFASSFileDialogueTextMakeChange(CFASSFileDialogueTextRef dialogueText, CFASSFileChangeRef change)
{
    if(dialogueText == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueText %p MakeChange %p", dialogueText, change);
    CFEnumeratorRef enumerator;
    CFASSFileDialogueTextContentRef eachTextContent = NULL;
    
    if(change->type & CFASSFileChangeTypeDiscardAllOverride)
    {
		bool checkPoint;
		size_t index, length;
		do
		{
			checkPoint = false;
			length = CFPointerArrayGetLength(dialogueText->contentArray);
			for (index = 0; index < length; index++)
			{
				eachTextContent = CFPointerArrayGetPointerAtIndex(dialogueText->contentArray, index);
				if (CFASSFileDialogueTextContentGetType(eachTextContent) == CFASSFileDialogueTextContentTypeOverride)
				{
					checkPoint = true;
					break;
				}
			}
			if (checkPoint)
			{
				CFPointerArrayRemovePointerAtIndex(dialogueText->contentArray, index, false);
				CFASSFileDialogueTextContentDestory(eachTextContent);
			}
		} while (checkPoint);
        if(CFPointerArrayGetLength(dialogueText->contentArray) == 0)
            CFPointerArrayAddPointer(dialogueText->contentArray, CFASSFileDialogueTextContentCreateEmptyString(), false);
    }
    enumerator = CFASSFileDialogueTextCreateEnumerator(dialogueText);
    while ((eachTextContent = CFEnumeratorNextObject(enumerator)) != NULL)
        CFASSFileDialogueTextContentMakeChange(eachTextContent, change);
    CFEnumeratorDestory(enumerator);
}

CFEnumeratorRef CFASSFileDialogueTextCreateEnumerator(CFASSFileDialogueTextRef dialogueText)
{
    if(dialogueText == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueText NULL CreateEnumerator");
    return CFEnumeratorCreateFromArray(dialogueText->contentArray);
}

void CFASSFileDialogueTextAddContent(CFASSFileDialogueTextRef dialogueText, CFASSFileDialogueTextContentRef content)
{
    if(dialogueText == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueText NULL AddContent");
        return;
    }
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextAddContent NULL");
        return;
    }
    CFPointerArrayAddPointer(dialogueText->contentArray, content, false);
}

CFASSFileDialogueTextRef CFASSFileDialogueTextCopy(CFASSFileDialogueTextRef dialogueText)
{
    CFASSFileDialogueTextRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueText))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            size_t arrayLength = CFPointerArrayGetLength(dialogueText->contentArray);
            bool copyCheck = true;
            CFASSFileDialogueTextContentRef eachContent;
            for (size_t index = 0; index<arrayLength && copyCheck; index++) {
                eachContent = CFASSFileDialogueTextContentCopy
                ((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(dialogueText->contentArray, index));
                if(eachContent == NULL)
                    copyCheck = false;
                else
                    CFPointerArrayAddPointer(result->contentArray, eachContent, false);
            }
            if(copyCheck)
                return result;
            arrayLength = CFPointerArrayGetLength(result->contentArray);
            for (size_t index = 0; index<arrayLength; index++)
                CFASSFileDialogueTextContentDestory
                ((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(result->contentArray, index));
            CFPointerArrayDestory(result->contentArray);
        }
        free(result);
    }
    return NULL;
}

int CFASSFileDialogueTextStoreStringResult(CFASSFileDialogueTextRef text, wchar_t * targetPoint)
{
    int result = 0, temp;
    size_t arrayLength = CFPointerArrayGetLength(text->contentArray);
    for(size_t index = 0; index<arrayLength; index++)
    {
        temp = CFASSFileDialogueTextContentStoreStringResult
        ((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(text->contentArray, index), targetPoint);
        if(temp == -1) return -1;
        if(targetPoint != NULL)
            targetPoint += temp;
        result += temp;
    }
    if(targetPoint != NULL)
    {
        targetPoint[0] = L'\n';
        targetPoint[1] = L'\0';
    }
    return result+1;
}

CFASSFileDialogueTextRef CFASSFileDialogueTextCreateWithString(const wchar_t *source)
{
    CFASSFileDialogueTextRef result;
    CFASSFileDialogueTextContentRef eachContent;
    
    const wchar_t *beginPoint, *endPoint;
    endPoint = wcsstr(source, L"\n");
    if(endPoint == NULL) endPoint = source + wcslen(source);
    if((result = malloc(sizeof(struct CFASSFileDialogueText))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
        {
            if(endPoint == source)
            {
                if((eachContent = CFASSFileDialogueTextContentCreateEmptyString()) != NULL)
                {
                    CFPointerArrayAddPointer(result->contentArray, eachContent, false);
                    return result;
                }
            }
            else
            {
                beginPoint = source;
                
                bool isFormatCorrect = true;
                CFASSFileDialogueTextContentType contentType;
                do
                {
                    if(*beginPoint == L'{') contentType = CFASSFileDialogueTextContentTypeOverride;
                    else contentType = CFASSFileDialogueTextContentTypeText;
                    if((eachContent = CFASSFileDialogueTextContentCreateWithString(contentType, beginPoint, endPoint - 1)) != NULL)
                    {
                        CFPointerArrayAddPointer(result->contentArray, eachContent, false);
                        if(*beginPoint == L'{')
                        {
                            beginPoint+=2;      // think about it
                            while (beginPoint<endPoint && *(beginPoint-1)!=L'}') beginPoint++;
                        }
                        else
                            while (beginPoint<endPoint && *beginPoint!= L'{') beginPoint++;
                    }
                    else isFormatCorrect = false;
                }while(isFormatCorrect && beginPoint < endPoint);
                
                if(isFormatCorrect)
                    return result;
            }
            size_t arrayLength = CFPointerArrayGetLength(result->contentArray);
            for(unsigned int index = 0; index<arrayLength; index++)
                CFASSFileDialogueTextContentDestory((CFASSFileDialogueTextContentRef)
                                                    CFPointerArrayGetPointerAtIndex(result->contentArray, index));
            CFPointerArrayDestory(result->contentArray);
        }
        free(result);
    }
    return NULL;
}

void CFASSFileDialogueTextDestory(CFASSFileDialogueTextRef dialogueText)
{
    size_t arrayLength = CFPointerArrayGetLength(dialogueText->contentArray);   
    for(unsigned int index = 0; index<arrayLength; index++)
        CFASSFileDialogueTextContentDestory((CFASSFileDialogueTextContentRef)CFPointerArrayGetPointerAtIndex(dialogueText->contentArray, index));
    CFPointerArrayDestory(dialogueText->contentArray);
    free(dialogueText);
}

void function(void)
{
    int temp = 0;
    if(temp == '{') return;
}
