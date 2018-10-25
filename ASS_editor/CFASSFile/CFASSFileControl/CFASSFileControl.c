//
//  CFASSFileControl.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/7.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stddef.h>
#include <wchar.h>

#include "CFASSFileControl.h"
#include "CFASSFileControl_Private.h"
#include "CFUseTool.h"
#include "CFException.h"

#define CFASSFileControlErrorOutputErrorSampleStringLengthMax 256

static CFASSFileControlErrorHandling CFASSFileControlGlobalErrorHandling = CFASSFileControlErrorHandlingOutput | CFASSFileControlErrorHandlingIgnore;

static FILE *redirectFile = NULL;

void CFASSFileControlErrorOutput(const wchar_t *fileContent, const wchar_t *errorPoint)
{
    int line = CF_find_line_number_in_wchar_text(fileContent, errorPoint);
    if(line<0)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileControlErrorOutput fileContent %p not contain errorPoint %p", fileContent, errorPoint);
    const wchar_t *errorEndPoint = errorPoint;
    while (*errorEndPoint!=L'\n'&&*errorEndPoint!=L'\0') errorEndPoint++;
    FILE *outDestination = stdout;
    if(redirectFile != NULL)
        outDestination = redirectFile;
    fwprintf(outDestination, L"CFASSFile ERROR LINE:%2d\n", line);
    if(errorEndPoint-errorPoint>CFASSFileControlErrorOutputErrorSampleStringLengthMax)
        errorEndPoint = errorPoint+CFASSFileControlErrorOutputErrorSampleStringLengthMax;
    if(errorEndPoint>errorPoint) errorEndPoint--;
    fwprintf(outDestination, L"SAMPLE: ");
    while(errorPoint<=errorEndPoint) putwc(*errorPoint++, outDestination);
    if(errorPoint[-1]!='\n')
        putwc(L'\n', outDestination);
}

CFASSFileControlErrorHandling CFASSFileControlGetErrorHandling(void)
{
    return CFASSFileControlGlobalErrorHandling;
}

void CFASSFileControlSetErrorHandling(CFASSFileControlErrorHandling errorHandle)
{
    CFASSFileControlGlobalErrorHandling = errorHandle;
}

bool CFASSFileRedirectErrorInfomation(FILE *fp)
{
    if(redirectFile == NULL && fp != NULL)
    {
        redirectFile = fp;
        return true;
    }
    return false;
}

bool CFASSFileUnRedirectErrorInfomation(void)
{
    if(redirectFile != NULL)
    {
        redirectFile = NULL;
        return false;
    }
    return true;
}
