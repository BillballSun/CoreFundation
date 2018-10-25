//
//  CFASSFileStyle_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/9.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStyle_Private_h
#define CFASSFileStyle_Private_h

#include "CFASSFileStyle.h"

#pragma mark - Read File

CFASSFileStyleRef CFASSFileStyleCreateWithString(const wchar_t *content);
/* content */
// Sample:
// Style: Name,FontName,25,&H00FFFFFF,&HF0000000,&H00000000,&HF0000000,-1,0,0,0,100,100,0,0.00,1,0,0,2,30,30,10,1

#pragma mark - Allocate Result

wchar_t *CFASSFileStyleAllocateFileContent(CFASSFileStyleRef style);

#endif /* CFASSFileStyle_Private_h */
