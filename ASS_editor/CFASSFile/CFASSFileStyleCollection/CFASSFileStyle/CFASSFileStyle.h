//
//  CFASSFileStyle.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/9.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStyle_h
#define CFASSFileStyle_h

#include "CFASSFileChange.h"

typedef struct CFASSFileStyleColor
{
    unsigned char alpha, blue, green, red;
} CFASSFileStyleColor;

#pragma mark - Create/Copy/Destory

CFASSFileStyleColor CFASSFileStyleColorMake(unsigned char alpha00,
                                            unsigned char blueFF,
                                            unsigned char greenFF,
                                            unsigned char redFF);

typedef struct CFASSFileStyle *CFASSFileStyleRef;

CFASSFileStyleRef CFASSFileStyleCreate(wchar_t *name,
                                       wchar_t *font_name, unsigned int font_size,
                                       CFASSFileStyleColor primary_colour,
                                       CFASSFileStyleColor secondary_colour,
                                       CFASSFileStyleColor outline_colour,
                                       bool blod, bool italic, bool underline, bool strike_out,
                                       double scale_x, double scale_y,
                                       double spacing,
                                       double angle,
                                       int border_style,
                                       unsigned int outline, unsigned int shadow,
                                       int alignment,
                                       unsigned int marginL, unsigned int marginR, unsigned marginV,
                                       unsigned encoding);

CFASSFileStyleRef CFASSFileStyleCopy(CFASSFileStyleRef style);

void CFASSFileStyleDestory(CFASSFileStyleRef style);

#pragma mark - Receive Change

void CFASSFileStyleMakeChange(CFASSFileStyleRef style, CFASSFileChangeRef change);

#endif /* CFASSFileStyle_h */
