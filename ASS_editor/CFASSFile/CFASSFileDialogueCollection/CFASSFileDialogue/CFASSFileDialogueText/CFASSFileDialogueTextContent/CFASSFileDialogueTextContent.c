//
//  CFASSFileDialogueTextContent.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <wchar.h>
#include <stdlib.h>

#include "CFUseTool.h"
#include "CFASSFileDialogueTextContent.h"
#include "CFASSFileDialogueTextContent_Private.h"
#include "CFASSFileDialogueTextContentOverride.h"
#include "CFASSFileDialogueTextContentOverride_Private.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"

struct CFASSFileDialogueTextContent
{
    CFASSFileDialogueTextContentType type;
    union
    {
        wchar_t *text;
        CFASSFileDialogueTextContentOverrideRef override;
    } data;
};

void CFASSFileDialogueTextContentMakeChange(CFASSFileDialogueTextContentRef textContent, CFASSFileChangeRef change)
{
    if(textContent == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContent %p MakeChange %p", textContent, change);
    if(textContent->type == CFASSFileDialogueTextContentTypeOverride &&
       CFASSFileChangeShouldDispatchToDialogueTextContentOverride(change))
        CFASSFileDialogueTextContentOverrideMakeChange(textContent->data.override, change);
    else
    {
        /* change applied to text type */
    }
}

CFASSFileDialogueTextContentType CFASSFileDialogueTextContentGetType(CFASSFileDialogueTextContentRef textContent)
{
    if(textContent == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContent NULL GetType");
    return textContent->type;
}

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithText(wchar_t *text, bool transferAllocationOwnership)
{
    if(text == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentCreateWithText NULL");
        return NULL;
    }
    CFASSFileDialogueTextContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContent))) != NULL)
    {
        if(transferAllocationOwnership)
        {
            result->type = CFASSFileDialogueTextContentTypeText;
            result->data.text = text;
            return result;
        }
        else if((result->data.text = CF_Dump_wchar_string(text)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentTypeText;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithOverride(CFASSFileDialogueTextContentOverrideRef override, bool transferOwnership)
{
    if(override == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentCreateWithOverride NULL");
        return NULL;
    }
    CFASSFileDialogueTextContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContent))) != NULL)
    {
        if(transferOwnership)
        {
            result->type = CFASSFileDialogueTextContentTypeOverride;
            result->data.override = override;
            return result;
        }
        else if((result->data.override = CFASSFileDialogueTextContentOverrideCopy(override)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentTypeOverride;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCopy(CFASSFileDialogueTextContentRef content)
{
    if(content == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContent NULL Copy");
        return NULL;
    }
    CFASSFileDialogueTextContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContent))) != NULL)
    {
        switch (result->type = content->type) {
            case CFASSFileDialogueTextContentTypeText:
                if((result->data.text = CF_Dump_wchar_string(content->data.text)) != NULL)
                    return result;
                break;
            case CFASSFileDialogueTextContentTypeOverride:
                if((result->data.override = CFASSFileDialogueTextContentOverrideCopy(content->data.override)) != NULL)
                    return result;
                break;
        }
        free(result);
    }
    return NULL;
}

static bool CFASSFileDialogueTextContentTextIsSkip(wchar_t test);

int CFASSFileDialogueTextContentStoreStringResult(CFASSFileDialogueTextContentRef textContent, wchar_t *targetPoint)
{
    if(textContent->type == CFASSFileDialogueTextContentTypeText)
    {
        if(targetPoint != NULL)
            wcscpy(targetPoint, textContent->data.text);
        return (int)wcslen(textContent->data.text);
    }
    else
        return CFASSFileDialogueTextContentOverrideStoreStringResult(textContent->data.override, targetPoint);
}

void CFASSFileDialogueTextContentDestory(CFASSFileDialogueTextContentRef content)
{
    switch (content->type) {
        case CFASSFileDialogueTextContentTypeText:
            free(content->data.text);
            break;
            
        case CFASSFileDialogueTextContentTypeOverride:
            CFASSFileDialogueTextContentOverrideDestory(content->data.override);
            break;
    }
    free(content);
}

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateEmptyString(void)
{
    CFASSFileDialogueTextContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContent))) != NULL)
    {
        if((result->data.text = malloc(sizeof(wchar_t))) != NULL)
        {
            *result->data.text = L'\0';
            result->type = CFASSFileDialogueTextContentTypeText;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFASSFileDialogueTextContentRef CFASSFileDialogueTextContentCreateWithString(CFASSFileDialogueTextContentType type,
                                                                             const wchar_t *data,
                                                                             const wchar_t *endPoint)
{
    CFASSFileDialogueTextContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContent))) != NULL)
    {
        switch(type)
        {
            case CFASSFileDialogueTextContentTypeText:
                if((result->data.text =
                    CF_allocate_wchar_string_with_endPoint_skip_terminate(data,
                                                                          endPoint,
                                                                          CFASSFileDialogueTextContentTextIsSkip,
                                                                          NULL)) != NULL)
                {
                    result->type = CFASSFileDialogueTextContentTypeText;
                    return result;
                }
                break;
            case CFASSFileDialogueTextContentTypeOverride:
                if(*data == L'{')
                {
                    wchar_t *temp = wcschr(data, L'}');
                    if(temp!=NULL && temp<=endPoint)
                        if((result->data.override = CFASSFileDialogueTextContentOverrideCreateWithString(data, true)) != NULL)
                        {
                            result->type = CFASSFileDialogueTextContentTypeOverride;
                            return result;
                        }
                }
                break;
        }
        free(result);
    }
    return NULL;
}

static bool CFASSFileDialogueTextContentTextIsSkip(wchar_t test)
{
    return test == L'{';
}
