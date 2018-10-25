//
//  CFASSFileStleCollection_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStyleCollection_Private_h
#define CFASSFileStyleCollection_Private_h

#include <wchar.h>

#include "CFASSFileStyleCollection.h"

#pragma mark - Read File

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCreateWithUnicodeFileContent(const wchar_t *content);

#pragma mark - Allocate Result

wchar_t *CFASSFileStyleCollectionAllocateFileContent(CFASSFileStyleCollectionRef styleCollection);

#pragma mark - Register Callbacks

int CFASSFileStyleCollectionRegisterAssociationwithFile(CFASSFileStyleCollectionRef scriptInfo, CFASSFileRef assFile);
/* Return */
// return zero means success, -1 means failed

#endif /* CFASSFileStyleCollection_Private_h */
