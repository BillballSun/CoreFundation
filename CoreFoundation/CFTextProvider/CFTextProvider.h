//
//  CFTextProvider.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFTextProvider_h
#define CFTextProvider_h

#include <stdio.h>

typedef struct CFTextProvider *CFTextProviderRef;

CFTextProviderRef CFTextProviderCreateWithFileUrl(const char *url);

CFTextProviderRef CFTextProviderCreateWithString(const char *string);

CFTextProviderRef CFTextProviderCreateWithWcharString(const wchar_t *string);

void CFTextProviderDestory(CFTextProviderRef textProvider);

#endif /* CFTextProvider_h */
