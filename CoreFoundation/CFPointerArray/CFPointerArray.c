//
//  CFPointerArray.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/10.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdlib.h>
#include <stdbool.h>

#include "CFPointerArray.h"
#include "CFPointerArray_Private.h"
#include "CFException.h"

#define CFPointerArrayIntializedCapacity 6
#define CFPointerArrayExtendCapacity 6

typedef struct CFPointerArrayItem
{
    void *pointer;
    bool hasOwnerShip;
} CFPointerArrayItem;

struct CFPointerArray
{
    size_t itemAmount;
    size_t arrayLength;
    CFPointerArrayItem *dataArray;
};

static bool CFPointerArrayChangeStorage(CFPointerArrayRef array, size_t newStorage);

static bool CFPointerArrayShrinkStorage(CFPointerArrayRef array);

size_t *CFPointerArrayGetItemAmountCheckPoint(CFPointerArrayRef array)
{
    if(array == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointerArray NULL GetItemAmountCheckPoint");
    return &array->itemAmount;
}

CFPointerArrayRef CFPointerArrayCopy(CFPointerArrayRef array, bool transferOwnership)
{
    CFPointerArrayRef result;
    if((result = malloc(sizeof(struct CFPointerArray))) != NULL)
    {
        size_t allocLength = array->itemAmount==0?1:array->itemAmount;
        if((result->dataArray = malloc(sizeof(struct CFPointerArrayItem) * allocLength)) != NULL)
        {
            for(size_t index = 0; index<array->itemAmount; index++)
            {
                result->dataArray[index].pointer = array->dataArray[index].pointer;
                if(transferOwnership && array->dataArray[index].hasOwnerShip)
                {
                    result->dataArray[index].hasOwnerShip = true;
                    array->dataArray[index].hasOwnerShip = false;
                }
                else
                    result->dataArray[index].hasOwnerShip = false;
            }
            result->arrayLength = allocLength;
            result->itemAmount = array->itemAmount;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFPointerArrayRef CFPointerArrayCreateEmpty(void)
{
    CFPointerArrayRef result;
    if((result = malloc(sizeof(struct CFPointerArray))) != NULL)
    {
        if((result->dataArray = malloc(sizeof(CFPointerArrayItem)*CFPointerArrayIntializedCapacity)) != NULL)
        {
            result->itemAmount = 0u;
            result->arrayLength = CFPointerArrayIntializedCapacity;
            return result;
        }
        free(result);
    }
    return NULL;
}

bool CFPointerArrayHasOwnershipAtIndex(CFPointerArrayRef array, size_t index)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer access ownership will NULL");
        return false;
    }
    if(index>=array->itemAmount)
    {
        CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "CFPointer %#X access index %lu with itemAmount %lu.", (unsigned int)array, (unsigned long)index, (unsigned long)array->itemAmount);
        return false;
    }
    return array->dataArray[index].hasOwnerShip;
}

size_t CFPointerArrayGetLength(CFPointerArrayRef array)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer access length will NULL");
        return (size_t)-1;
    }
    return array->itemAmount;
}

void CFPointerArrayAddPointer(CFPointerArrayRef array, void *pointer, bool transferOwnership)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer NULL add pointer %X", (unsigned int)pointer);
        return;
    }
    if(pointer == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer %X add pointer NULL", (unsigned int)array);
        return;
    }
    if(array->itemAmount >= array->arrayLength)
        if(!CFPointerArrayChangeStorage(array, array->arrayLength+CFPointerArrayExtendCapacity))
        {
            CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "Allocation for CFPointerArray %#X failed", (unsigned int)array);
            return;
        }
    array->dataArray[array->itemAmount].pointer = pointer;
    array->dataArray[array->itemAmount++].hasOwnerShip = transferOwnership;
}

static bool CFPointerArrayChangeStorage(CFPointerArrayRef array, size_t newStorage)
{
    if(newStorage<array->itemAmount || newStorage == 0)
        return false;
    CFPointerArrayItem *newData = realloc(array->dataArray, newStorage * sizeof(struct CFPointerArrayItem));
    if(newData != NULL)
    {
        array->arrayLength = newStorage;
        array->dataArray = newData;
        return true;
    }
    return false;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

static bool CFPointerArrayShrinkStorage(CFPointerArrayRef array)
{
    size_t newStorage = array->itemAmount == 0?1:array->itemAmount;
    if(newStorage==1 && array->arrayLength == 1)
        return false;
    CFPointerArrayItem *newData = realloc(array->dataArray, newStorage * sizeof(struct CFPointerArrayItem));
    if(newData != NULL)
    {
        array->arrayLength = newStorage;
        array->dataArray = newData;
        return true;
    }
    return false;
}

#pragma clang diagnostic pop

void *CFPointerArrayRemovePointerAtIndex(CFPointerArrayRef array, size_t index, bool transferOwnership)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer NULL remove pointer at index %u", (unsigned int)index);
        return NULL;
    }
    if(index>=array->itemAmount)
    {
        CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "CFPointer %#X remove index %lu with itemAmount %lu.", (unsigned int)array, (unsigned long)index, (unsigned long)array->itemAmount);
        return NULL;
    }
    void *returnValue = NULL;
    if(array->dataArray[index].hasOwnerShip)
    {
        if(transferOwnership)
            returnValue = array->dataArray[index].pointer;
        else
            free(array->dataArray[index].pointer);
    }
    for(size_t currentIndex = index; currentIndex<array->itemAmount-1; currentIndex++)
        array->dataArray[currentIndex] = array->dataArray[currentIndex+1];
    array->itemAmount--;
    return returnValue;
}

size_t CFPointerArrayInsertPointerAtIndex(CFPointerArrayRef array, void *pointer, size_t index, bool transferOwnership)
{
    if(index>array->itemAmount) index = array->itemAmount;
    if(array->itemAmount+1>array->arrayLength)
        if(!CFPointerArrayChangeStorage(array, array->arrayLength+CFPointerArrayExtendCapacity))
        {
            CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "Allocation for CFPointerArray %#X failed", (unsigned int)array);
            return (size_t)-1;
        }
    for(size_t currentIndex = array->itemAmount; currentIndex>index; currentIndex--)
        array->dataArray[currentIndex] = array->dataArray[currentIndex-1];
    array->dataArray[index].pointer = pointer;
    array->dataArray[index].hasOwnerShip = transferOwnership;
    array->itemAmount++;
    return index;
}

void CFPointerArraySwapPointerPosition(CFPointerArrayRef array, size_t index1, size_t index2)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer NULL swap index%u and index%u", (unsigned int)index1, (unsigned int)index2);
        return;
    }
    if(index1>=array->itemAmount || index2>=array->itemAmount)
    {
        CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "CFPointer %#X swap index %lu %lu with itemAmount %lu.", (unsigned int)array, (unsigned long)index1, (unsigned long)index2, (unsigned long)array->itemAmount);
        return;
    }
    if(index1 == index2) return;
    CFPointerArrayItem temp = array->dataArray[index1];
    array->dataArray[index1] = array->dataArray[index2];
    array->dataArray[index2] = temp;
}

void *CFPointerArrayGetPointerAtIndex(CFPointerArrayRef array, size_t index)
{
    if(array == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFPointer NULL get pointer at index %u", (unsigned int)index);
        return NULL;
    }
    if(index>=array->itemAmount)
    {
        CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "CFPointer %#X get index %lu with itemAmount %lu.", (unsigned int)array, (unsigned long)index, (unsigned long)array->itemAmount);
        return NULL;
    }
    return array->dataArray[index].pointer;
}

void *CFPointerArrayGetLastPointer(CFPointerArrayRef array)
{
    if(array->itemAmount == 0)
    {
        CFExceptionRaise(CFExceptionNameArrayOutOfBounds, NULL, "CFPointer %#X get pointer with itemAmount 0.", (unsigned int)array);
        return NULL;
    }
    return CFPointerArrayGetPointerAtIndex(array, array->itemAmount-1);
}

void CFPointerArrayDestory(CFPointerArrayRef array)
{
    for(size_t index = 0; index<array->itemAmount; index++)
        if(array->dataArray[index].hasOwnerShip)
            free(array->dataArray[index].pointer);
    free(array->dataArray);
    free(array);
}



















































