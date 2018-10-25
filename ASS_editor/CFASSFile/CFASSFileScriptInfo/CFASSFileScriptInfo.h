//
//  CFASSFileScriptInfo.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileScriptInfo_h
#define CFASSFileScriptInfo_h

#include "CFUseTool.h"
#include "CFASSFileChange.h"

typedef struct CFASSFileScriptInfo *CFASSFileScriptInfoRef;

#pragma mark - Create/Copy/Destory

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateEssential(wchar_t *title,
                                                          wchar_t *original_script,
                                                          bool is_collisions_normal,
                                                          unsigned int play_res_x,
                                                          unsigned int play_res_y,
                                                          double timePercentage);

CFASSFileScriptInfoRef CFASSFileScriptInfoCopy(CFASSFileScriptInfoRef scriptInfo);

void CFASSFileScriptInfoDestory(CFASSFileScriptInfoRef scriptInfo);

#pragma mark - Get Info

CFUSize CFASSFileScriptInfoGetResolutionXY(CFASSFileScriptInfoRef scriptInfo);

#pragma mark - Receive Change

void CFASSFileScriptInfoMakeChange(CFASSFileScriptInfoRef scriptInfo, CFASSFileChangeRef change);

#endif /* CFASSFileScriptInfo_h */
