//
//  CFDebug.c
//  ASS_Editor
//
//  Created by Bill Sun on 2018/10/24.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "CFDebug.h"

void CFDebugLog(const char * restrict format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}
