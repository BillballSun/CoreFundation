//
//  UTF32String.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/10/1.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "UTF32String.h"
#include "CFException.h"

#define UTF8Mask0(mask) (!(((UTF8Char)(1 << 7)) & (mask)))
#define UTF8Mask10(mask) (!(((UTF8Char)(1 << 6)) & (mask)))
#define UTF8Mask110(mask) (!(((UTF8Char)(1 << 5)) & (mask)))
#define UTF8Mask1110(mask) (!(((UTF8Char)(1 << 4)) & (mask)))
#define UTF8Mask11110(mask) (!(((UTF8Char)(1 << 3)) & (mask)))
#define UTF8Mask111110(mask) (!(((UTF8Char)(1 << 2)) & (mask)))
#define UTF8Mask1111110(mask) (!(((UTF8Char)(1 << 1)) & (mask)))

struct UTF32String {
    size_t length;
    UTF32Char data[];   // size is (length+1), malloc(sizeof(struct UTF32String) + sizeof(UTF32Char) * (length + 1))
};

static UTF32StringRef UTF32StringCreateWithLength(size_t length);

UTF32StringRef UTF32StringCreateWithCharacterArray(UTF32Char *array, size_t length)
{
    UTF32StringRef result;
    if((result = UTF32StringCreateWithLength(length)) != NULL)
    {
        for(size_t index = 0; index < length; index++)
            result->data[index] = array[index];
        result->data[length] = '\0';
        return result;
    }
    return NULL;
}

UTF32StringRef UTF32StringCopy(UTF32StringRef from)
{
    UTF32StringRef result;
    if((result = malloc(sizeof(struct UTF32String) + sizeof(UTF32Char) * (from->length + 1))) != NULL)
    {
        memcpy(result, from, sizeof(struct UTF32String) + sizeof(UTF32Char) * (from->length + 1));
        return result;
    }
    return NULL;
}

UTF32Char UTF32StringGetCharAtIndex(UTF32StringRef string, size_t index)
{
    if(string == NULL || index >= string->length)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "UTF32StringGetCharAtIndex string %s index %lu with length %lu", string == NULL ? "NULL" : "", (unsigned long)index, string == NULL ? (unsigned long)0 : (unsigned long)string->length);
    return string->data[index];
}

size_t UTF32StringGetLength(UTF32StringRef string)
{
    if(string == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "UTF32StringGetCharAtIndex string NULL");
    return string->length;
}

UTF32StringRef UTF32StringCreateWithUTF8String(const UTF8Char *string)
{
    if(string == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "UTF32StringCreateWithUTF8String NULL");
    size_t length = 0;
    bool errorFlag = false; size_t errorIndex = 0;
    for(size_t index = 0; string[index] != '\0'; index++)
        if(UTF8Mask0(string[index]))
            length++;
        else if(UTF8Mask10(string[index]))
        {
            printf("%x", (int)string[index]);
            errorFlag = true;
            errorIndex = index;
            break;
        }
        else if(UTF8Mask110(string[index]))
        {
            // 110xxxxx 10xxxxxx
            for (size_t check = 0; check < 1; check++)
            {
                index++;
                if(UTF8Mask0(string[index]) || !UTF8Mask10(string[index]))
                {
                    errorFlag = true;
                    errorIndex = index;
                    break;
                }
            }
            if(errorFlag) break;
            length++;
        }
        else if(UTF8Mask1110(string[index]))
        {
            // 110xxxxx 10xxxxxx 10xxxxxx
            for (size_t check = 0; check < 2; check++)
            {
                index++;
                if(UTF8Mask0(string[index]) || !UTF8Mask10(string[index]))
                {
                    errorFlag = true;
                    errorIndex = index;
                    break;
                }
            }
            if(errorFlag) break;
            length++;
        }
        else if(UTF8Mask11110(string[index]))
        {
            // 110xxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            for (size_t check = 0; check < 3; check++)
            {
                index++;
                if(UTF8Mask0(string[index]) || !UTF8Mask10(string[index]))
                {
                    errorFlag = true;
                    errorIndex = index;
                    break;
                }
            }
            if(errorFlag) break;
            length++;
        }
        else if(UTF8Mask111110(string[index]))
        {
            // 110xxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            for (size_t check = 0; check < 4; check++)
            {
                index++;
                if(UTF8Mask0(string[index]) || !UTF8Mask10(string[index]))
                {
                    errorFlag = true;
                    errorIndex = index;
                    break;
                }
            }
            if(errorFlag) break;
            length++;
        }
        else if(UTF8Mask1111110(string[index]))
        {
            // 110xxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            for (size_t check = 0; check < 5; check++)
            {
                index++;
                if(UTF8Mask0(string[index]) || !UTF8Mask10(string[index]))
                {
                    errorFlag = true;
                    errorIndex = index;
                    break;
                }
            }
            if(errorFlag) break;
            length++;
        }
        else
        {
            errorFlag = true;
            errorIndex = index;
            break;
        }
    if(errorFlag)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "UTF32StringCreateWithUTF8String with Invalid UTF8 String at index %lu", (unsigned long)errorIndex);
        return NULL;
    }
    UTF32StringRef result;
    if((result = UTF32StringCreateWithLength(length)) != NULL)
    {
        size_t dataIndex = 0;
        for(size_t index = 0; string[index] != '\0'; index++)
            if(UTF8Mask0(string[index]))
            {
                result->data[dataIndex++] = (UTF32Char)string[index];
            }
            else if(UTF8Mask110(string[index]))
            {
                // 110xxxxx 10xxxxxx
                result->data[dataIndex++] = ((UTF32Char)((UTF8Char)0x1F & string[index]) << 6)
                                          +  (UTF32Char)((UTF8Char)0x3F & string[index+1]);
                index += 1;
            }
            else if(UTF8Mask1110(string[index]))
            {
                // 1110xxxx 10xxxxxx 10xxxxxx
                result->data[dataIndex++] = ((UTF32Char)((UTF8Char)0x0F & string[index]) << 12)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+1]) << 6)
                                          +  (UTF32Char)((UTF8Char)0x3F & string[index+2]);
                index += 2;
            }
            else if(UTF8Mask11110(string[index]))
            {
                // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                result->data[dataIndex++] = ((UTF32Char)((UTF8Char)0x07 & string[index]) << 18)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+1]) << 12)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+2]) << 6)
                                          +  (UTF32Char)((UTF8Char)0x3F & string[index+3]);
                index += 3;
            }
            else if(UTF8Mask111110(string[index]))
            {
                // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                result->data[dataIndex++] = ((UTF32Char)((UTF8Char)0x03 & string[index]) << 24)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+1]) << 18)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+2]) << 12)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+3]) << 6)
                                          +  (UTF32Char)((UTF8Char)0x3F & string[index+4]);
                index += 4;
            }
            else if(UTF8Mask1111110(string[index]))
            {
                // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
                result->data[dataIndex++] = ((UTF32Char)((UTF8Char)0x01 & string[index]) << 30)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+1]) << 24)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+2]) << 18)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+3]) << 12)
                                          + ((UTF32Char)((UTF8Char)0x3F & string[index+4]) << 6)
                                          +  (UTF32Char)((UTF8Char)0x3F & string[index+5]);
                index += 5;
            }
        result->data[dataIndex] = '\0';
        return result;
    }
    return NULL;
}

static UTF32StringRef UTF32StringCreateWithLength(size_t length)
{
    UTF32StringRef result;
    if((result = malloc(sizeof(struct UTF32String) + sizeof(UTF32Char) * (length + 1))) != NULL)
    {
        result->length = length;
        return result;
    }
    return NULL;
}

void UTF32StringDestory(UTF32StringRef string)
{
    if(string == NULL) return;
    free(string);
}
