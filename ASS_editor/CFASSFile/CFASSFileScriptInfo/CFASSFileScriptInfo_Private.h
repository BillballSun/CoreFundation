//
//  CFASSFileScriptInfo_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileScriptInfo_Private_h
#define CFASSFileScriptInfo_Private_h

#include <wchar.h>

#include "CFASSFileScriptInfo.h"

#pragma mark - Read File

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateWithUnicodeFileContent(const wchar_t *content);

#pragma mark - Allocate Result

wchar_t *CFASSFileScriptInfoAllocateFileContent(CFASSFileScriptInfoRef scriptInfo);

#pragma mark - Register Callbacks

int CFASSFileScriptInfoRegisterAssociationwithFile(CFASSFileScriptInfoRef scriptInfo, CFASSFileRef assFile);
/* Return */
// return zero means success, -1 means failed

#endif /* CFASSFileScriptInfo_Private_h */
