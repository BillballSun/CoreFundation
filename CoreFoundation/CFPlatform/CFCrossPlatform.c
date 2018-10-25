//
//  CFCrossPlatform.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/7.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <locale.h>

#include "CFPlatform.h"
#include "CFCrossPlatform.h"

#ifdef CF_OS_WIN
#include <fcntl.h>
#endif

void CFCrossPlatformUnicodeSupport(void)
{
#ifdef CF_OS_WIN
    _setmode(_fileno(stdout), _O_U16TEXT);
#elif defined CF_OS_MAC
    setlocale(LC_CTYPE, "zh_CN");
    // for a list of available locales, please terminal "locale -a"
#endif
}
