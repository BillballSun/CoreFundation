//
//  CFException.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/18.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <signal.h>
#include <stdarg.h>

#include "CFException.h"
#include "CFPointerArray.h"

const char * const CFExceptionNameArrayOutOfBounds = "CFException: OutOfBounds";

const char * const CFExceptionNameDivideZero = "CFException: DivideZero";

const char * const CFExceptionNameOverflow = "CFException: Overflow";

const char * const CFExceptionNameInvalidArgument = "CFException: InvalidArgument";

const char * const CFExceptionNameProcessFailed = "CFException: ProcessFailed";

static CFPointerArrayRef CFExceptionStack = NULL;

#pragma mark - Function

static void CFExceptionStackInitialization(void);

static bool CFExceptionDefaultCatchFunction(const char * exceptionName, void * data);


void CFExceptionRaise(const char * exceptionName, void * data, const char * format, ...)
{
    fprintf(stderr, "CFException Raised - ");
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    if(CFExceptionStack == NULL) CFExceptionStackInitialization();
    size_t arrayLength = CFPointerArrayGetLength(CFExceptionStack);
    bool handled = false;
    for(size_t count = arrayLength; count>=1; count--)
        if((handled = ((CFExceptionCatchFunction)CFPointerArrayGetPointerAtIndex(CFExceptionStack, count-1))(exceptionName, data)))
            break;
    if(!handled)
        CFExceptionDefaultCatchFunction(exceptionName, data);
}

static void CFExceptionStackInitialization(void)
{
    if(CFExceptionStack == NULL)
        CFExceptionStack = CFPointerArrayCreateEmpty();
}

void CFExceptionCatchStackPush(CFExceptionCatchFunction function)
{
    if(CFExceptionStack == NULL) CFExceptionStackInitialization();
    CFPointerArrayAddPointer(CFExceptionStack, function, false);
}

CFExceptionCatchFunction CFExceptionCatchStackPop(void)
{
    if(CFExceptionStack == NULL) CFExceptionStackInitialization();
    size_t arrayLength = CFPointerArrayGetLength(CFExceptionStack);
    if(arrayLength==0) return NULL;
    CFExceptionCatchFunction result = CFPointerArrayGetPointerAtIndex(CFExceptionStack, arrayLength-1);
    CFPointerArrayRemovePointerAtIndex(CFExceptionStack, arrayLength-1, false);
    return result;
}

CFExceptionCatchFunction CFExceptionCatchStackCurrentFunction(void)
{
    if(CFExceptionStack == NULL) CFExceptionStackInitialization();
    size_t arrayLength = CFPointerArrayGetLength(CFExceptionStack);
    CFExceptionCatchFunction result;
    if(arrayLength==0)
        result = CFExceptionDefaultCatchFunction;
    else
        result = CFPointerArrayGetPointerAtIndex(CFExceptionStack, arrayLength-1);
    return result;
}

static bool CFExceptionDefaultCatchFunction(const char * exceptionName, void * data)
{
    raise(SIGABRT);
    return true;
}



















































