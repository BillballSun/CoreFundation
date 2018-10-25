//
//  CFASSFileDialogueTextDrawingContext.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/13.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextDrawingContext_h
#define CFASSFileDialogueTextDrawingContext_h

#include <stdbool.h>

typedef struct CFASSFileDialogueTextDrawingContext *CFASSFileDialogueTextDrawingContextRef;

CFASSFileDialogueTextDrawingContextRef CFASSFileDialogueTextDrawingContextCreate(void);

void CFASSFileDialogueTextDrawingContextDestory(CFASSFileDialogueTextDrawingContextRef context);

void CFASSFileDialogueTextDrawingContextMoveToPosition(CFASSFileDialogueTextDrawingContextRef context,
                                                       int x, int y, bool closing);

void CFASSFileDialogueTextDrawingContextDrawLine(CFASSFileDialogueTextDrawingContextRef context, int x, int y);

void CFASSFileDialogueTextDrawingContextDrawBezier(CFASSFileDialogueTextDrawingContextRef context,
                                                   int x1, int y1,
                                                   int x2, int y2,
                                                   int x3, int y3);

void CFASSFileDialogueTextDrawingContextDrawBSpline(CFASSFileDialogueTextDrawingContextRef context, unsigned int degrees, ...);

void CFASSFileDialogueTextDrawingContextExtendBSpline(CFASSFileDialogueTextDrawingContextRef context,
                                                      int x, int y,
                                                      bool attachToPreviousBSpline);

void CFASSFileDialogueTextDrawingContextClose(CFASSFileDialogueTextDrawingContextRef context);

#endif /* CFASSFileDialogueTextDrawing_h */
