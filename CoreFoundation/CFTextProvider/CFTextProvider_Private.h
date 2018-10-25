//
//  CFTextProvider_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFTextProvider_Private_h
#define CFTextProvider_Private_h

#include <wchar.h>

#include "CFTextProvider.h"

wchar_t *CFTextProviderAllocateTextContentwithUnicodeEncoding(CFTextProviderRef provider, size_t *length);

#endif /* CFTextProvider_Private_h */
