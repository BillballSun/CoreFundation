//
//  CFUseTool.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#include "CFUseTool.h"
#include "CFException.h"

CFUSize CFUSizeMake(unsigned int x, unsigned int y)
{
    return (CFUSize){x, y};
}

CFSize CFSizeMake(int x, int y)
{
    return (CFSize){x, y};
}

const wchar_t *CF_wcsstr_with_end_point(const wchar_t *sourceString, const wchar_t *findString, const wchar_t *endPoint)
{
    size_t findStringLength = wcslen(findString);
    if(endPoint!=NULL) endPoint -= findStringLength-1;
    while((endPoint==NULL?1:sourceString<=endPoint) && *sourceString!=L'\0')
        if(wmemcmp(sourceString, findString, findStringLength)) sourceString++;
        else return sourceString;
    return NULL;
}

wchar_t *CF_Dump_wchar_string(const wchar_t *source)
{
    if(source==NULL) return NULL;
    size_t length = wcslen(source);
    wchar_t *result = malloc(sizeof(wchar_t)*(length+1));
    if(result!=NULL)
        wmemcpy(result, source, length+1);
    return result;
}

wchar_t *CF_allocate_wchar_string_with_endPoint_skip_terminate(const wchar_t *sourceString,
                                                               const wchar_t *endPoint,
                                                               bool (*isSkip)(wchar_t),
                                                               bool *terminateBecauseOfSkip)
{
    if(sourceString == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF_allocate_wchar_with string NULL");
        return NULL;
    }
    if(endPoint != NULL && endPoint<sourceString)
        return NULL;
    const wchar_t *tokenBegin = sourceString, *tokenEnd;
    bool continueLoop, allocZero;
    continueLoop = true;
    allocZero = false;
    do
    {
        if(*tokenBegin == L'\0')
        {
            continueLoop = false;
            allocZero = true;
        }
        else if(endPoint != NULL && tokenBegin == endPoint)
        {
            continueLoop = false;
            if(isSkip!=NULL && isSkip(*tokenBegin))
                allocZero = true;
        }
        else
        {
            if(isSkip!=NULL && isSkip(*tokenBegin))
                tokenBegin++;
            else
                continueLoop = false;
        }
    }while(continueLoop);
    if(allocZero)
    {
        wchar_t *result = malloc(sizeof(wchar_t)*(1));
        if(result!=NULL)
        {
            result[0] = L'\0';
            return result;
        }
    }
    else
    {
        tokenEnd = tokenBegin;
        continueLoop = true;
        do
        {
            if(*tokenEnd == L'\0')
            {
                tokenEnd--;
                continueLoop = false;
            }
            else if(endPoint!=NULL && tokenEnd == endPoint)
            {
                if(isSkip!=NULL && isSkip(*tokenEnd))
                    tokenEnd--;
                continueLoop = false;
            }
            else if(isSkip!=NULL && isSkip(*tokenEnd))
            {
                tokenEnd--;
                continueLoop = false;
            }
            else
                tokenEnd++;
        }while (continueLoop);
        wchar_t *result = malloc(sizeof(wchar_t)*(tokenEnd-tokenBegin+1+1));
        if(result!=NULL)
        {
            wmemcpy(result, tokenBegin, tokenEnd-tokenBegin+1);
            result[tokenEnd-tokenBegin+1] = L'\0';
            return result;
        }
    }
    return NULL;
}

bool CF_wchar_string_match_beginning(const wchar_t *testString,
                                     const wchar_t *match)
{
    while(*match != L'\0')
        if(*match++ != *testString++) return false;
    return true;
}

const wchar_t *CF_match_next_braces(const wchar_t *beginBrace, const wchar_t *endPoint, bool loopInto)
{
    if(beginBrace == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF_match_next braces NULL");
        return NULL;
    }
    wchar_t matchBrace;
    if(*beginBrace == L'(')
        matchBrace = L')';
    else if(*beginBrace == L'[')
        matchBrace = L']';
    else if(*beginBrace == L'{')
        matchBrace = L'}';
    else
        return NULL;
    if(loopInto)
    {
        beginBrace++;
        if(beginBrace>endPoint)
            return NULL;
        while(*beginBrace!=matchBrace
              && (endPoint == NULL?true:beginBrace<=endPoint)
              && *beginBrace!=L'\0')
        {
            if(*beginBrace==L'(' || *beginBrace==L'[' || *beginBrace==L'{')
            {
                const wchar_t *loopMatch;
                if((loopMatch = CF_match_next_braces(beginBrace, endPoint, true)) == NULL)
                    return NULL;
                else
                    beginBrace = loopMatch;
            }
            beginBrace++;
        }
        if(*beginBrace == matchBrace)
            return beginBrace;
    }
    else
    {
        while(*beginBrace!=matchBrace && (endPoint == NULL?true:beginBrace<=endPoint) && *beginBrace!=L'\0')
            beginBrace++;
        if(*beginBrace == matchBrace)
            return beginBrace;
    }
    return NULL;
}

long CF_mbslen(const char *mbs)
{
    if(mbs == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF_mbslen NULL parameter");
        return -1;
    }
    const char *begin = mbs;
    int temp;
    while((temp = mblen(mbs, MB_LEN_MAX)) > 0)
        mbs += temp;
    if(temp<0) return -1L;
    else return mbs - begin;
}

long CF_multibyte_character_amount(const char *mbs)
{
    if(mbs == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF_multibyte_character_amount NULL parameter");
        return -1L;
    }
    long result = 0;
    int temp;
    while((temp = mblen(mbs, MB_LEN_MAX)) > 0)
    {
        mbs += temp;
        result++;
    }
    if(temp<0) return -1L;
    else return result;
}

wchar_t *CF_Dump_mbs_to_wcs(const char *mbs)
{
    if(mbs == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF_Dump_mbs_to_wcs NULL parameter");
        return NULL;
    }
    long stringLength = CF_multibyte_character_amount(mbs);
    if(stringLength >= 0)
    {
        wchar_t *result;
        if((result = malloc(sizeof(wchar_t)*(stringLength+1))) != NULL)
        {
            if(mbstowcs(result, mbs, stringLength+1) <= stringLength)
                return result;
            free(result);
        }
    }
    return NULL;
}

CFTextEncoding CF_check_text_encoding(const char *string)
{
    if(string == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF check text NULL encoding");
    if(memcmp(string, "\xEF\xBB\xBF", 3) == 0)
        return CFTextEncodingUTF8;
    if(memcmp(string, "\x00\x00\xFE\xFF", 4) == 0)
        return CFTextEncodingUTF32BE;
    if(memcmp(string, "\xFF\xFE\x00\x00", 4) == 0)
        return CFTextEncodingUTF32LE;
    if(memcmp(string, "\xFE\xFF", 2) == 0)
        return CFTextEncodingUTF16BE;
    if(memcmp(string, "\xFF\xFE", 2) == 0)
        return CFTextEncodingUTF16LE;
    return CFTextEncodingUnkown;
}

int CF_find_line_number_in_wchar_text(const wchar_t *text, const wchar_t *line)
{
    if(text == NULL || line == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF find line %p number in wchar text %p", line, text);
    int currentLine = 1;
    bool checkPoint = true;
    do
    {
        if(text == line)
            return currentLine;
        if(*text == L'\0')
            checkPoint = false;
        else if(*text == L'\n')
            currentLine++;
        text++;
    }while (checkPoint);
    return -1;
}

wchar_t *CF_allocate_text_translation_from_winStyle_to_unixStyle(const wchar_t *winText)
{
    if(winText == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CF allocate text NULL translation from winStyle to unixStyle");
    size_t stringLength = 0;
    const wchar_t *current = winText;
    while(*current!=L'\0')
    {
        if(current[0]==L'\r' && current[1]==L'\n')
            current++;
        current++;
        stringLength++;
    }
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t) * (stringLength+1))) != NULL)
    {
        current = winText;
        size_t index = 0;
        while(*current!=L'\0')
        {
            if(current[0]==L'\r' && current[1]==L'\n')
                current++;
            result[index++] = *current++;
        }
        result[index] = L'\0';
        return result;
    }
    return NULL;
}






























