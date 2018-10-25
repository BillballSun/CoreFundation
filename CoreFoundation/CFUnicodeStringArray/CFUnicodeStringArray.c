//
//  CFUnicodeStringArray.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <wchar.h>

#include "CFUnicodeStringArray.h"
#include "CFPointerArray.h"
#include "CFUseTool.h"
#include "CFException.h"

struct CFUnicodeStringArray
{
    CFPointerArrayRef stringArray;
};

CFUnicodeStringArrayRef CFUnicodeStringArrayCopy(CFUnicodeStringArrayRef array)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL copy");
        return NULL;
    }
    CFUnicodeStringArrayRef result;
    const wchar_t *eachString;
    if((result = CFUnicodeStringArrayCreateEmpty()) != NULL)
    {
        size_t arrayLength = CFUnicodeStringArrayGetLength(array);
        bool copyCheck = true;
        for(size_t index = 0; index<arrayLength && copyCheck; index++)
            if((eachString = CFUnicodeStringArrayGetStringAtIndex(array, index)) != NULL)
                CFUnicodeStringArrayAddString(result, eachString, false);    /* Not dumped */
            else
                copyCheck = false;
        if(copyCheck)
            return result;
        CFUnicodeStringArrayDestory(result);
    }
    return NULL;
}

CFUnicodeStringArrayRef CFUnicodeStringArrayCreateEmpty(void)
{
    CFUnicodeStringArrayRef result;
    if((result = malloc(sizeof(struct CFUnicodeStringArray))) != NULL)
    {
        if((result->stringArray = CFPointerArrayCreateEmpty()) != NULL)
            return result;
        free(result);
    }
    return NULL;
}

size_t CFUnicodeStringArrayGetLength(CFUnicodeStringArrayRef array)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL get length");
        return (size_t)-1;
    }
    return CFPointerArrayGetLength(array->stringArray);
}

size_t CFUnicodeStringArrayAddString(CFUnicodeStringArrayRef array, const wchar_t * string, bool transferOwnership)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL add string");
        return (size_t)-1;
    }
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray %X add string NULL", (unsigned int)array);
        return (size_t)-1;
    }
    size_t stringLength = wcslen(string);
    if(transferOwnership)
    {
        CFPointerArrayAddPointer(array->stringArray, (void *)string, true);
        return stringLength;
    }
    else
    {
        wchar_t *copyedString;
        if((copyedString = malloc(sizeof(wchar_t)*(stringLength+1))) != NULL)
        {
            wcscpy(copyedString, string);
            CFPointerArrayAddPointer(array->stringArray, copyedString, true);
            return stringLength;
        }
    }
    return (size_t)-1;
}

void CFUnicodeStringArrayRemoveStringAtIndex(CFUnicodeStringArrayRef array, size_t index)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL remove index %u", (unsigned int)index);
        return;
    }
    CFPointerArrayRemovePointerAtIndex(array->stringArray, index, false);
}

size_t CFUnicodeStringArrayInsertStringAtIndex(CFUnicodeStringArrayRef array, const wchar_t *string, size_t index)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL insert string at index %u", (unsigned int)index);
        return (size_t)-1;
    }
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray %X insert string NULL at index %u", (unsigned int)array, (unsigned int)index);
        return (size_t)-1;
    }
    size_t stringLength = wcslen(string);
    wchar_t *copyedString;
    if((copyedString = malloc(sizeof(wchar_t)*(stringLength+1))) != NULL)
    {
        wcscpy(copyedString, string);
        CFPointerArrayInsertPointerAtIndex(array->stringArray, copyedString, index, true);
        return stringLength;
    }
    return (size_t)-1;
}

void CFUnicodeStringArraySwapStringPosition(CFUnicodeStringArrayRef array, size_t index1, size_t index2)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL swap index %u and index %u", (unsigned int)index1, (unsigned)index2);
        return;
    }
    CFPointerArraySwapPointerPosition(array->stringArray, index1, index2);
}

size_t CFUnicodeStringArrayAddStringWithEndChar(CFUnicodeStringArrayRef array, const wchar_t *string, const wchar_t endChar, bool *encounterEndChar)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL add string with end char %lc", endChar);
        return (size_t)-1;
    }
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray %X add string NULL with end char %lc", (unsigned int)array, endChar);
        return (size_t)-1;
    }
    const wchar_t *endPoint = string;
    while(*endPoint!=L'\0' && *endPoint!=endChar) endPoint++;
    if(encounterEndChar!=NULL)
    {
        if(*endPoint == endChar)
            *encounterEndChar = true;
        else
            *encounterEndChar = false;
    }
    size_t stringLength = endPoint-string;
    wchar_t *copyedString;
    if((copyedString = malloc(sizeof(wchar_t)*(stringLength+1))) != NULL)
    {
        wmemcpy(copyedString, string, stringLength);
        copyedString[stringLength] = L'\0';
        CFPointerArrayAddPointer(array->stringArray, copyedString, true);
        return stringLength;
    }
    return (size_t)-1;
}

const wchar_t *CFUnicodeStringArrayGetStringAtIndex(CFUnicodeStringArrayRef array, size_t index)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFUnicodeStringArray NULL get string at index %u", (unsigned int)index);
        return NULL;
    }
    return (const wchar_t *)CFPointerArrayGetPointerAtIndex(array->stringArray, index);
}

void CFUnicodeStringArrayDestory(CFUnicodeStringArrayRef array)
{
    if(array == NULL) return;
    CFPointerArrayDestory(array->stringArray);
    free(array);
}



















































