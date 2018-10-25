//
//  CFASSFileControl.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/7.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileControl_h
#define CFASSFileControl_h

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    CFASSFileControlErrorHandlingIgnore = 1 << 0,
    CFASSFileControlErrorHandlingOutput = 1 << 1,
}CFASSFileControlErrorHandling;

CFASSFileControlErrorHandling CFASSFileControlGetErrorHandling(void);

void CFASSFileControlSetErrorHandling(CFASSFileControlErrorHandling errorHandle);

bool CFASSFileRedirectErrorInfomation(FILE *fp);
// if success, return true

bool CFASSFileUnRedirectErrorInfomation(void);
// if success, return true

#endif /* CFASSFileControl_h */
