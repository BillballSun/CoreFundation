//
//  CFASSFile_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/9.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFile_Private_h
#define CFASSFile_Private_h

#include "CFASSFile.h"
#include "CFASSFileScriptInfo.h"
#include "CFASSFileStyleCollection.h"
#include "CFUseTool.h"
#include "CFGeometry.h"

#pragma mark - Get component

CFASSFileScriptInfoRef CFASSFileGetScriptInfo(CFASSFileRef file);

CFASSFileStyleCollectionRef CFASSFileGetStyleCollection(CFASSFileRef file);

CFASSFileDialogueCollectionRef CFASSFileGetDialogueCollection(CFASSFileRef file);

#pragma mark - Receive Callbacks

void CFASSFileReceiveResolutionXYChangeFromScriptInfo(CFASSFileRef file, CFUSize from, CFUSize to);

#endif /* CFASSFile_Private_h */
