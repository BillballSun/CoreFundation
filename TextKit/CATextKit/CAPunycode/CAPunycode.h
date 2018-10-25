//
//  CAPunycode.h
//  TextKit
//
//  Created by Bill Sun on 2018/10/23.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CAPunycode_h
#define CAPunycode_h

#include "UTF32String.h"

UTF32StringRef CAPunycodeDecodeASCIIString(const UTF8Char *source);

UTF32StringRef CAPunycodeEecodeUnicodeString(const UTF32Char *source);

#endif /* CAPunycode_h */
