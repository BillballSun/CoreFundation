//
//  CFTextProvider.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "CFPlatform.h"
#ifdef CF_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "CFTextProvider.h"
#include "CFTextProvider_Private.h"
#include "CFUseTool.h"
#include "CFException.h"

struct CFTextProvider
{
    wchar_t *allocatedString;
};

void CFTextProviderDestory(CFTextProviderRef textProvider)
{
    if(textProvider == NULL) return;
    free(textProvider->allocatedString);
    free(textProvider);
}

CFTextProviderRef CFTextProviderCreateWithFileUrl(const char *url)
{
#ifdef CF_OS_WIN
    FILE *fp;
    if((fp = fopen(url, "rt, ccs=UNICODE")) != NULL)
    {
        fpos_t beginPosition;
        if(fgetpos(fp, &beginPosition) == 0)
        {
            size_t fileLength = 0;
            while(getwc(fp)!=WEOF) fileLength++;
            if(fsetpos(fp, &beginPosition) == 0)
            {
                wchar_t *allocatedFileString;
                if((allocatedFileString = malloc(sizeof(wchar_t)*(fileLength+1))) != NULL)
                {
                    wchar_t *currentPoint = allocatedFileString;
                    while((*currentPoint++ = getwc(fp)) != WEOF) continue;
                    currentPoint[-1] = L'\0';
                    CFTextProviderRef result;
                    if((result = malloc(sizeof(struct CFTextProvider))) != NULL)
                    {
                        result->allocatedString = allocatedFileString;
                        fclose(fp);
                        return result;
                    }
                    free(allocatedFileString);
                }
            }
        }
        fclose(fp);
    }
    return NULL;
#elif defined(CF_OS_MAC)
    FILE *fp;
    if((fp = fopen(url, "rb")) != NULL)
    {
        size_t fileLength = 0;
        while(getc(fp)!=EOF) fileLength++;
        unsigned char *allocated;
        if((allocated = malloc(fileLength)))    /* No need for \0 */
        {
            rewind(fp); 
            int eachCharacter;
            size_t index = 0;
            while((eachCharacter = getc(fp)) != EOF)
                allocated[index++] = eachCharacter;
            
            /* string check */
            CFStringEncoding stringEncoding;
            Boolean isExternalRepresentation = true;
            CFTextEncoding checkEncoding = CF_check_text_encoding((const char *)allocated);
            
            if(checkEncoding == CFTextEncodingUTF8)
                stringEncoding = kCFStringEncodingUTF8;
            else if(checkEncoding == CFTextEncodingUTF16BE)
                stringEncoding = kCFStringEncodingUTF16BE;
            else if(checkEncoding == CFTextEncodingUTF16LE)
                stringEncoding = kCFStringEncodingUTF16LE;
            else if(checkEncoding == CFTextEncodingUTF32BE)
                stringEncoding = kCFStringEncodingUTF32BE;
            else if(checkEncoding == CFTextEncodingUTF32LE)
                stringEncoding = kCFStringEncodingUTF32LE;
            else
            {
                stringEncoding = kCFStringEncodingUTF8;
                isExternalRepresentation = false;
            }
            
            CFStringRef string = CFStringCreateWithBytes(kCFAllocatorDefault, allocated, fileLength, stringEncoding, isExternalRepresentation);
            if(string!=NULL)
            {
                CFIndex length = CFStringGetLength(string);
                wchar_t *getFromCFString;
                if((getFromCFString = malloc(sizeof(wchar_t) * (length+1))) != NULL)
                {
                    Boolean checkMark = CFStringGetCString(string, (char *)getFromCFString, sizeof(wchar_t) * (length+1), kCFStringEncodingUTF32LE);
                    if(checkMark)
                    {
                        if(CF_check_text_encoding((const char *)getFromCFString)!=CFTextEncodingUnkown)
                            wmemmove(getFromCFString, getFromCFString+1, length);
                        wchar_t *temp = realloc(getFromCFString, sizeof(wchar_t) * length);
                        if(temp!=NULL) getFromCFString = temp;
                        
                        wchar_t *translateToUnixStyle = CF_allocate_text_translation_from_winStyle_to_unixStyle(getFromCFString);
                        if(translateToUnixStyle!=NULL)
                        {
                            CFTextProviderRef textProvider;
                            if((textProvider = malloc(sizeof(struct CFTextProvider))) != NULL)
                            {
                                CFRelease(string);
                                free(allocated);
                                free(getFromCFString);
                                fclose(fp);
                                textProvider->allocatedString = translateToUnixStyle;
                                return textProvider;
                            }
                        }
                    }
                    free(getFromCFString);
                }
                CFRelease(string);
            }
            free(allocated);
        }
        fclose(fp);
    }
    return NULL;
#endif
}

CFTextProviderRef CFTextProviderCreateWithString(const char *string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFTextProviderCreateWithString NULL");
        return NULL;
    }
    CFTextProviderRef result;
    if((result = malloc(sizeof(struct CFTextProvider))) != NULL)
    {
        if((result->allocatedString = CF_Dump_mbs_to_wcs(string)) != NULL)
            return result;
        free(result);
    }
    return NULL;
}

CFTextProviderRef CFTextProviderCreateWithWcharString(const wchar_t *string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFTextProviderCreateWithString NULL");
        return NULL;
    }
    CFTextProviderRef result;
    if((result = malloc(sizeof(struct CFTextProvider))) != NULL)
    {
        if((result->allocatedString = CF_Dump_wchar_string(string)) != NULL)
            return result;
        free(result);
    }
    return NULL;
}

wchar_t *CFTextProviderAllocateTextContentwithUnicodeEncoding(CFTextProviderRef provider, size_t *length)
{
    if(provider == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFTextProvider NULL AllocateTextContentwithUnicodeEncoding");
        return NULL;
    }
    wchar_t *result = CF_Dump_wchar_string(provider->allocatedString);
    if(result!=NULL)
    {
        if(length!=NULL) *length = wcslen(result);
        return result;
    }
    return NULL;
}



















































