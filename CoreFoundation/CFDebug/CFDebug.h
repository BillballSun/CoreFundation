//
//  CFDebug.h
//  ASS_Editor
//
//  Created by Bill Sun on 2018/10/24.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFDebug_h
#define CFDebug_h

void CFDebugLog(const char * restrict format, ...)
__attribute__((format(printf, 1, 2)));

#endif /* CFDebug_h */
