//
//  CFEnumerator.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/6.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>

#include "CFEnumerator.h"
#include "CFEnumerator_Private.h"
#include "CFPointerArray.h"
#include "CFPointerArray_Private.h"
#include "CFException.h"

struct CFEnumerator
{
    size_t nextIndex;
    size_t allCount, *countCheckPoint;  // if allCount is zero, nextIndex MUST be 1
    CFPointerArrayRef data;
};

CFEnumeratorRef CFEnumeratorCreate(CFPointerArrayRef data, size_t *countCheckPoint, bool transferArrayOwnership)
{
    if(data == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFEnumeratorCreate pass NULL as data argument");
    if(countCheckPoint!=NULL && CFPointerArrayGetLength(data)!=*countCheckPoint)
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CFEnumeratorCreate countCheckPoint does not match initial array length");
    CFEnumeratorRef result;
    if((result = malloc(sizeof(struct CFEnumerator))) != NULL)
    {
        if(transferArrayOwnership ||
           (data = CFPointerArrayCopy(data, false))!=NULL)
        {
            if((result->allCount = CFPointerArrayGetLength(data)) == 0)
                result->nextIndex = 1;
            else
                result->nextIndex = 0;
            result->countCheckPoint = countCheckPoint;
            result->data = data;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFEnumeratorRef CFEnumeratorCreateFromArray(CFPointerArrayRef array)
{
    CFPointerArrayRef copied;
    if((copied = CFPointerArrayCopy(array, false))!=NULL)
        return CFEnumeratorCreate(copied, CFPointerArrayGetItemAmountCheckPoint(array), true);
    return NULL;
}

void *CFEnumeratorNextObject(CFEnumeratorRef enumerator)
{
    if(enumerator == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFEnumerator NULL NextObject");
    if(enumerator->nextIndex>=enumerator->allCount)
        return NULL;
    if(enumerator->countCheckPoint!=NULL && *enumerator->countCheckPoint!=enumerator->allCount)
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CFEnumeratorNextObject, previous data changed");
    return CFPointerArrayGetPointerAtIndex(enumerator->data, enumerator->nextIndex++);
}

CFPointerArrayRef CFEnumeratorGetRemainedObjects(CFEnumeratorRef enumerator)
{
    if(enumerator == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFEnumerator NULL GetRemainedObjects");
    CFPointerArrayRef result;
    if((result = CFPointerArrayCreateEmpty()) != NULL)
    {
        void *each;
        while((each = CFEnumeratorNextObject(enumerator)) != NULL)
            CFPointerArrayAddPointer(result, each, false);
        return result;
    }
    return NULL;
}

void CFEnumeratorDestory(CFEnumeratorRef enumerator)
{
    if(enumerator == NULL) return;
    CFPointerArrayDestory(enumerator->data);
    free(enumerator);
}
