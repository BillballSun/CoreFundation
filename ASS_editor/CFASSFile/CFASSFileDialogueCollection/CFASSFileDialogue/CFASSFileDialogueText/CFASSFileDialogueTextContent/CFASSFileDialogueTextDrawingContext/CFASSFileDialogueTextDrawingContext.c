//
//  CFASSFileDialogueTextDrawingContext.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/13.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <wchar.h>

#include "CFASSFileDialogueTextDrawingContext.h"
#include "CFASSFileDialogueTextDrawingContext_Private.h"
#include "CFPointerArray.h"
#include "CFException.h"

#pragma mark context

struct CFASSFileDialogueTextDrawingContext {
    CFPointerArrayRef contentArray;
};

#pragma mark - contextContent

typedef enum CFASSFileDialogueTextDrawingContextContentType {
    CFASSFileDialogueTextDrawingContextContentTypeMove,
    CFASSFileDialogueTextDrawingContextContentTypeLine,
    CFASSFileDialogueTextDrawingContextContentTypeBezier,
    CFASSFileDialogueTextDrawingContextContentTypeBSpline,
    CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline,
    CFASSFileDialogueTextDrawingContextContentTypeCloseBSpline
} CFASSFileDialogueTextDrawingContextContentType;

struct CFASSFileDialogueTextDrawingContextContent {
    CFASSFileDialogueTextDrawingContextContentType type;
    void *data;
};

typedef struct CFASSFileDialogueTextDrawingContextContent *CFASSFileDialogueTextDrawingContextContentRef;

struct CFASSFileDialogueTextDrawingContextMoveContent {
    int x, y;
    bool closePath;
};

struct CFASSFileDialogueTextDrawingContextLineContent {
    int x, y;
};

struct CFASSFileDialogueTextDrawingContextBezierContent {
    int x1, y1,
        x2, y2,
        x3, y3;
};

struct CFASSFileDialogueTextDrawingContextBSplineContent {
    int x1, y1,
        x2, y2,
        x3, y3;
};

struct CFASSFileDialogueTextDrawingContextExtendBSplineContent {
    bool attachToPreviousBSplineContent;
    int x, y;
};

typedef struct CFASSFileDialogueTextDrawingContextContent *CFASSFileDialogueTextDrawingContextContentRef;

typedef struct CFASSFileDialogueTextDrawingContextMoveContent *CFASSFileDialogueTextDrawingContextMoveContentRef;

typedef struct CFASSFileDialogueTextDrawingContextLineContent *CFASSFileDialogueTextDrawingContextLineContentRef;

typedef struct CFASSFileDialogueTextDrawingContextBezierContent *CFASSFileDialogueTextDrawingContextBezierContentRef;

typedef struct CFASSFileDialogueTextDrawingContextBSplineContent *CFASSFileDialogueTextDrawingContextBSplineContentRef;

typedef struct CFASSFileDialogueTextDrawingContextExtendBSplineContent *CFASSFileDialogueTextDrawingContextExtendBSplineContentRef;

static const wchar_t *CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(const wchar_t *beginPoint, unsigned int elementAmount);

#pragma mark - ContentManagement

CFASSFileDialogueTextDrawingContextRef CFASSFileDialogueTextDrawingContextCreate(void)
{
    CFASSFileDialogueTextDrawingContextRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextDrawingContext))) != NULL)
    {
        if((result->contentArray = CFPointerArrayCreateEmpty()) != NULL)
            return result;
        free(result);
    }
    return NULL;
}

void CFASSFileDialogueTextDrawingContextDestory(CFASSFileDialogueTextDrawingContextRef context)
{
    size_t length = CFPointerArrayGetLength(context->contentArray);
    CFASSFileDialogueTextDrawingContextContentRef content;
    for(size_t index = 0; index<length; index++)
    {
        content = CFPointerArrayGetPointerAtIndex(context->contentArray, index);
        // if(content->type != CFASSFileDialogueTextDrawingContextContentTypeCloseBSpline)  as it is assigned to NULL
        free(content->data);
        free(content);
    }
    CFPointerArrayDestory(context->contentArray);
    free(context);
}

void CFASSFileDialogueTextDrawingContextMoveToPosition(CFASSFileDialogueTextDrawingContextRef context,
                                                       int x, int y, bool closing)
{
    CFASSFileDialogueTextDrawingContextContentRef content;
    if((content = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextContent))) != NULL)
    {
        if((content->data = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextMoveContent))) != NULL)
        {
            content->type = CFASSFileDialogueTextDrawingContextContentTypeMove;
            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->closePath = closing;
            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->x = x;
            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y = y;
            CFPointerArrayAddPointer(context->contentArray, content, false);
            return;
        }
        free(content);
    }
}

void CFASSFileDialogueTextDrawingContextDrawLine(CFASSFileDialogueTextDrawingContextRef context, int x, int y)
{
    CFASSFileDialogueTextDrawingContextContentRef content;
    if((content = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextContent))) != NULL)
    {
        if((content->data = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextLineContent))) != NULL)
        {
            content->type = CFASSFileDialogueTextDrawingContextContentTypeLine;
            ((CFASSFileDialogueTextDrawingContextLineContentRef)content->data)->x = x;
            ((CFASSFileDialogueTextDrawingContextLineContentRef)content->data)->y = y;
            CFPointerArrayAddPointer(context->contentArray, content, false);
            return;
        }
        free(content);
    }
}

void CFASSFileDialogueTextDrawingContextDrawBezier(CFASSFileDialogueTextDrawingContextRef context,
                                                   int x1, int y1,
                                                   int x2, int y2,
                                                   int x3, int y3)
{
    CFASSFileDialogueTextDrawingContextContentRef content;
    if((content = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextContent))) != NULL)
    {
        if((content->data = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextBezierContent))) != NULL)
        {
            content->type = CFASSFileDialogueTextDrawingContextContentTypeBezier;
            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x1 = x1;
            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y1 = y1;
            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x2 = x2;
            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y2 = y2;
            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x3 = x3;
            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y3 = y3;
            CFPointerArrayAddPointer(context->contentArray, content, false);
            return;
        }
        free(content);
    }
}

void CFASSFileDialogueTextDrawingContextDrawBSpline(CFASSFileDialogueTextDrawingContextRef context, unsigned int degrees, ...)
{
    if(degrees<3) return;
    CFASSFileDialogueTextDrawingContextContentRef content;
    if((content = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextContent))) != NULL)
    {
        if((content->data = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextBSplineContent))) != NULL)
        {
            content->type = CFASSFileDialogueTextDrawingContextContentTypeBSpline;
            va_list ap;
            va_start(ap, degrees);
            ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x1 = va_arg(ap, int);
            ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y1 = va_arg(ap, int);
            ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x2 = va_arg(ap, int);
            ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y2 = va_arg(ap, int);
            ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x3 = va_arg(ap, int);
            ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y3 = va_arg(ap, int);
            CFPointerArrayAddPointer(context->contentArray, content, false);
            for(unsigned int currentDegree = 4; currentDegree<=degrees; currentDegree++)
                CFASSFileDialogueTextDrawingContextExtendBSpline(context, va_arg(ap, int), va_arg(ap, int), true);
            va_end(ap);
            return;
        }
        free(content);
    }
}

void CFASSFileDialogueTextDrawingContextExtendBSpline(CFASSFileDialogueTextDrawingContextRef context,
                                                      int x, int y,
                                                      bool attachToPreviousBSpline)
{
    CFASSFileDialogueTextDrawingContextContentType previousType = ((CFASSFileDialogueTextDrawingContextContentRef)CFPointerArrayGetLastPointer(context->contentArray))->type;
    if(previousType!=CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline &&
       previousType!=CFASSFileDialogueTextDrawingContextContentTypeBSpline)
        return;
    CFASSFileDialogueTextDrawingContextContentRef content;
    if((content = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextContent))) != NULL)
    {
        if((content->data = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextExtendBSplineContent))) != NULL)
        {
            content->type = CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline;
            ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x = x;
            ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y = y;
            ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->attachToPreviousBSplineContent = attachToPreviousBSpline;
            CFPointerArrayAddPointer(context->contentArray, content, false);
            return;
        }
        free(content);
    }
}

void CFASSFileDialogueTextDrawingContextClose(CFASSFileDialogueTextDrawingContextRef context)
{
    CFASSFileDialogueTextDrawingContextContentRef content;
    if((content = malloc(sizeof(struct CFASSFileDialogueTextDrawingContextContent))) != NULL)
    {
        content->type = CFASSFileDialogueTextDrawingContextContentTypeCloseBSpline;
        content->data = NULL;       // significant to assign NULL, otherwise destory function will crack
        CFPointerArrayAddPointer(context->contentArray, content, false);
        free(content);
    }
}

wchar_t *CFASSFileDialogueTextDrawingContextAllocateString(CFASSFileDialogueTextDrawingContextRef context)
{
    if(CFASSFileDialogueTextDrawingContextCheckValidation(context))
    {
        FILE *fp;
        if((fp = tmpfile())!=NULL)
        {
            size_t contentAmount = CFPointerArrayGetLength(context->contentArray);
            CFASSFileDialogueTextDrawingContextContentRef content;
            size_t stringLength = 0;
            int temp;
            bool creationSuccess = true;
            
            CFASSFileDialogueTextDrawingContextContentType previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
            bool previousExtendBSplineAttached = false;
            for(size_t index = 0; index<contentAmount && creationSuccess; index++)
            {
                content = CFPointerArrayGetPointerAtIndex(context->contentArray, 1);
                switch (content->type) {
                    case CFASSFileDialogueTextDrawingContextContentTypeMove:
                        if(((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->closePath)
                            temp = fwprintf(fp, L"m %d %d ",
                                            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->x,
                                            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                        else
                            temp = fwprintf(fp, L"n %d %d ",
                                            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->x,
                                            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                        previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
                        break;
                    case CFASSFileDialogueTextDrawingContextContentTypeLine:
                        if(index!=0 && previousType == CFASSFileDialogueTextDrawingContextContentTypeLine)
                            temp = fwprintf(fp, L"%d %d ",
                                            ((CFASSFileDialogueTextDrawingContextLineContentRef)content->data)->x,
                                            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                        else
                        {
                            temp = fwprintf(fp, L"l %d %d ",
                                            ((CFASSFileDialogueTextDrawingContextLineContentRef)content->data)->x,
                                            ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                            previousType = CFASSFileDialogueTextDrawingContextContentTypeLine;
                        }
                        break;
                    case CFASSFileDialogueTextDrawingContextContentTypeBezier:
                        if(index!=0 && previousType == CFASSFileDialogueTextDrawingContextContentTypeBezier)
                            temp = fwprintf(fp, L"%d %d %d %d %d %d ",
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x1,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y1,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x2,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y2,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x3,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y3);
                        else
                        {
                            temp = fwprintf(fp, L"b %d %d %d %d %d %d ",
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x1,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y1,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x2,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y2,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x3,
                                            ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y3);
                            previousType = CFASSFileDialogueTextDrawingContextContentTypeBezier;
                        }
                        break;
                    case CFASSFileDialogueTextDrawingContextContentTypeBSpline:
                        temp = fwprintf(fp, L"b %d %d %d %d %d %d ",
                                        ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x1,
                                        ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y1,
                                        ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x2,
                                        ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y2,
                                        ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x3,
                                        ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y3);
                        previousType = CFASSFileDialogueTextDrawingContextContentTypeBSpline;
                        break;
                    case CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline:
                        if(((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->attachToPreviousBSplineContent)
                        {
                            temp = fwprintf(fp, L"%d %d ",
                                            ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x,
                                            ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y);
                            previousExtendBSplineAttached = true;
                        }
                        else
                        {
                            
                            if(previousType == CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline && !previousExtendBSplineAttached)
                                temp = fwprintf(fp, L"%d %d ",
                                                ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x,
                                                ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y);
                            else
                                temp = fwprintf(fp, L"p %d %d ",
                                                ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x,
                                                ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y);
                            previousExtendBSplineAttached = false;
                        }
                        previousType = CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline;
                        break;
                    case CFASSFileDialogueTextDrawingContextContentTypeCloseBSpline:
                        temp = fwprintf(fp, L"c ");
                        break;
                }
                if(temp<0) creationSuccess = false;
                else stringLength += temp;
            }
            fclose(fp);
            if(creationSuccess)
            {
                stringLength--; // blank spacing remaining
                wchar_t *result;
                if((result = malloc(sizeof(wchar_t)*(stringLength+1))) != NULL)
                {
                    wchar_t *beginPoint = result;
                    size_t currentRemainLength = stringLength;
                    
                    previousExtendBSplineAttached = false;
                    previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
                    for(size_t index = 0; index<contentAmount && creationSuccess; index++)
                    {
                        content = CFPointerArrayGetPointerAtIndex(context->contentArray, 1);
                        switch (content->type) {
                            case CFASSFileDialogueTextDrawingContextContentTypeMove:
                                if(((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->closePath)
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"m %d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->x,
                                                    ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                                else
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"n %d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->x,
                                                    ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                                previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
                                break;
                            case CFASSFileDialogueTextDrawingContextContentTypeLine:
                                if(index!=0 && previousType == CFASSFileDialogueTextDrawingContextContentTypeLine)
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"%d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextLineContentRef)content->data)->x,
                                                    ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                                else
                                {
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"l %d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextLineContentRef)content->data)->x,
                                                    ((CFASSFileDialogueTextDrawingContextMoveContentRef)content->data)->y);
                                    previousType = CFASSFileDialogueTextDrawingContextContentTypeLine;
                                }
                                break;
                            case CFASSFileDialogueTextDrawingContextContentTypeBezier:
                                if(index!=0 && previousType == CFASSFileDialogueTextDrawingContextContentTypeBezier)
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"%d %d %d %d %d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x1,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y1,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x2,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y2,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x3,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y3);
                                else
                                {
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"b %d %d %d %d %d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x1,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y1,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x2,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y2,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->x3,
                                                    ((CFASSFileDialogueTextDrawingContextBezierContentRef)content->data)->y3);
                                    previousType = CFASSFileDialogueTextDrawingContextContentTypeBezier;
                                }
                                break;
                            case CFASSFileDialogueTextDrawingContextContentTypeBSpline:
                                temp = swprintf(beginPoint, currentRemainLength+1, L"b %d %d %d %d %d %d ",
                                                ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x1,
                                                ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y1,
                                                ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x2,
                                                ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y2,
                                                ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->x3,
                                                ((CFASSFileDialogueTextDrawingContextBSplineContentRef)content->data)->y3);
                                previousType = CFASSFileDialogueTextDrawingContextContentTypeBSpline;
                                break;
                            case CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline:
                                if(((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->attachToPreviousBSplineContent)
                                {
                                    temp = swprintf(beginPoint, currentRemainLength+1, L"%d %d ",
                                                    ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x,
                                                    ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y);
                                    previousExtendBSplineAttached = true;
                                }
                                else
                                {
                                    
                                    if(previousType == CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline && !previousExtendBSplineAttached)
                                        temp = swprintf(beginPoint, currentRemainLength+1, L"%d %d ",
                                                        ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x,
                                                        ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y);
                                    else
                                        temp = swprintf(beginPoint, currentRemainLength+1, L"p %d %d ",
                                                        ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->x,
                                                        ((CFASSFileDialogueTextDrawingContextExtendBSplineContentRef)content->data)->y);
                                    previousExtendBSplineAttached = false;
                                }
                                previousType = CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline;
                                break;
                            case CFASSFileDialogueTextDrawingContextContentTypeCloseBSpline:
                                temp = swprintf(beginPoint, currentRemainLength+1, L"c ");
                                break;
                        }
                        beginPoint += temp;
                        currentRemainLength -= temp;
                    }
                    return result;
                }
            }
        }
    }
    return NULL;
}

bool CFASSFileDialogueTextDrawingContextCheckValidation(CFASSFileDialogueTextDrawingContextRef context)
{
    CFPointerArrayRef array = context->contentArray;
    CFASSFileDialogueTextDrawingContextContentRef content;
    size_t length = CFPointerArrayGetLength(context->contentArray);
    if(length <= 1)
        return false;
    if((content = CFPointerArrayGetPointerAtIndex(array, 1))->type != CFASSFileDialogueTextDrawingContextContentTypeMove)
        return false;
    CFASSFileDialogueTextDrawingContextContentType previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
    for(size_t index = 2; index<length; index++)
    {
        content = CFPointerArrayGetPointerAtIndex(array, 1);
        if(content->type == CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline &&
           previousType!=CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline &&
           previousType!=CFASSFileDialogueTextDrawingContextContentTypeBSpline)
            return false;
        previousType = content->type;
    }
    return true;
}

CFASSFileDialogueTextDrawingContextRef CFASSFileDialogueTextDrawingContextCreateFromString(const wchar_t *string)
{
    if(string == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextDrawingContextCreateFromString NULL");
    if(string == L'\0')
        return NULL;
    CFASSFileDialogueTextDrawingContextRef result;
    if((result = CFASSFileDialogueTextDrawingContextCreate()) != NULL)
    {
        const wchar_t *beginPoint = string;
        int temp1, temp2, temp3, temp4, temp5, temp6;
        CFASSFileDialogueTextDrawingContextContentType previousType = CFASSFileDialogueTextDrawingContextContentTypeLine;
        bool firstDrawingCode = true;
        bool formatCheck = true;
        while (*beginPoint!=L'\0' && formatCheck) {
            if(swscanf(beginPoint, L"m %d %d", &temp1, &temp2) == 2)
            {
                CFASSFileDialogueTextDrawingContextMoveToPosition(result, temp1, temp2, true);
                previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
                beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 3);
            }
            else if(swscanf(beginPoint, L"n %d %d", &temp1, &temp2) == 2)
            {
                CFASSFileDialogueTextDrawingContextMoveToPosition(result, temp1, temp2, false);
                previousType = CFASSFileDialogueTextDrawingContextContentTypeMove;
                beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 3);
            }
            else if(swscanf(beginPoint, L"l %d %d", &temp1, &temp2) == 2)
            {
                CFASSFileDialogueTextDrawingContextDrawLine(result, temp1, temp2);
                previousType = CFASSFileDialogueTextDrawingContextContentTypeLine;
                beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 3);
            }
            else if(swscanf(beginPoint, L"b %d %d %d %d %d %d", &temp1, &temp2, &temp3, &temp4, &temp5, &temp6) == 6)
            {
                CFASSFileDialogueTextDrawingContextDrawBezier(result, temp1, temp2, temp3, temp4, temp5, temp6);
                previousType = CFASSFileDialogueTextDrawingContextContentTypeBezier;
                beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 7);
            }
            else if(swscanf(beginPoint, L"s %d %d %d %d %d %d", &temp1, &temp2, &temp3, &temp4, &temp5, &temp6) == 6)
            {
                CFASSFileDialogueTextDrawingContextDrawBSpline(result, temp1, temp2, temp3, temp4, temp5, temp6);
                previousType = CFASSFileDialogueTextDrawingContextContentTypeBSpline;
                beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 7);
            }
            else if(swscanf(beginPoint, L"p %d %d", &temp1, &temp2) == 2)
            {
                CFASSFileDialogueTextDrawingContextExtendBSpline(result, temp1, temp2, false);
                previousType = CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline;
                beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 3);
            }
            else if(!firstDrawingCode)
            {
                if(previousType == CFASSFileDialogueTextDrawingContextContentTypeLine &&
                   swscanf(beginPoint, L"%d %d", &temp1, &temp2) == 2)
                {
                    CFASSFileDialogueTextDrawingContextDrawLine(result, temp1, temp2);
                    previousType = CFASSFileDialogueTextDrawingContextContentTypeLine;
                    beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 2);
                }
                else if(previousType == CFASSFileDialogueTextDrawingContextContentTypeBezier &&
                        swscanf(beginPoint, L"%d %d %d %d %d %d", &temp1, &temp2, &temp3, &temp4, &temp5, &temp6) == 6)
                {
                    CFASSFileDialogueTextDrawingContextDrawBezier(result, temp1, temp2, temp3, temp4, temp5, temp6);
                    previousType = CFASSFileDialogueTextDrawingContextContentTypeBezier;
                    beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 6);
                }
                else if(previousType == CFASSFileDialogueTextDrawingContextContentTypeBSpline &&
                        swscanf(beginPoint, L"%d %d", &temp1, &temp2) == 2)
                {
                    CFASSFileDialogueTextDrawingContextExtendBSpline(result, temp1, temp2, true);
                    previousType = CFASSFileDialogueTextDrawingContextContentTypeBSpline;
                    beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 2);
                }
                else if(previousType == CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline &&
                        swscanf(beginPoint, L"%d %d", &temp1, &temp2) == 2)
                {
                    CFASSFileDialogueTextDrawingContextExtendBSpline(result, temp1, temp2, false);
                    previousType = CFASSFileDialogueTextDrawingContextContentTypeExtendBSpline;
                    beginPoint = CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(beginPoint, 2);
                }
                else
                    formatCheck = false;
            }
            else
                formatCheck = false;
            if(firstDrawingCode && previousType!=CFASSFileDialogueTextDrawingContextContentTypeMove)
                formatCheck = false;
            firstDrawingCode = false;
        }
        if(formatCheck)
            return result;
        CFASSFileDialogueTextDrawingContextDestory(result);
    }
    return NULL;
}

static const wchar_t *CFASSFileDialogueTextDrawingContextCreateFromStringSkipElement(const wchar_t *beginPoint, unsigned int elementAmount)
{
    if(beginPoint == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextDrawingContextCreateFromString NULL SkipElement");
    const wchar_t *search;
    const wchar_t *unfoundResult = beginPoint + wcslen(beginPoint);
    for(unsigned int count = 1; count<=elementAmount; count++)
    {
        search = wcschr(beginPoint, L' ');
        if(search == NULL)
            return unfoundResult;
        beginPoint = search;
        while(*beginPoint==L' ') beginPoint++;
        /* if now beginPoint is '\0', it is OK */
    }
    return beginPoint;
}
