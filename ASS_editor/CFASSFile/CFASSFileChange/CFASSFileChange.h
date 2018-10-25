//
//  CFASSFileChange.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/6.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileChange_h
#define CFASSFileChange_h

#include <stdbool.h>
#include "CFGeometry.h"

typedef struct CFASSFileChange *CFASSFileChangeRef;

/* Add new Change note:
 * 1. create change function
 * 2. CombineInto function
 * 3. Destory function
 * 4. Disptach function
 * 5. Somewhere in ASSFile structure to apply this change
 */

#pragma mark - Create/Copy/Combine/Destory

CFASSFileChangeRef CFASSFileChangeBorderStyle(int borderStyle);
// currently only 1 and 3 is correct, otherwise NULL returned

CFASSFileChangeRef CFASSFileChangeOutlineWidth(unsigned int outlinePixelWidth, bool forceBorderStyle);
// if forceBorderStyle, will force borderStyle to 1, and apply change to all
// otherwise, all borderStyle is 1, is apply to this change
// shadowPixelWidth overflowed 4, will be downgrade to 4

CFASSFileChangeRef CFASSFileChangeShadowWidth(unsigned int shadowPixelWidth, bool forceBorderStyle);
// if forceBorderStyle, will force borderStyle to 1, and apply change to all
// otherwise, all borderStyle is 1, is apply to this change
// shadowPixelWidth overflowed 4, will be downgrade to 4

CFASSFileChangeRef CFASSFileChangePrimaryColor(unsigned char redFF, unsigned char greenFF, unsigned char blueFF, unsigned char alpha00);

CFASSFileChangeRef CFASSFileChangeOutlineColor(unsigned char redFF, unsigned char greenFF, unsigned char blueFF, unsigned char alpha00);

CFASSFileChangeRef CFASSFileChangeDiscardAllOverride(void);

CFASSFileChangeRef CFASSFileChangeResolution(CFUSize newSize);

CFASSFileChangeRef CFASSFileChangeFontSize(bool usingPercentage, double percentage, unsigned int newSize);

CFASSFileChangeRef CFASSFileChangeFontName(wchar_t *fontName, bool affectOverride);

CFASSFileChangeRef CFASSFileChangeTimeOffset(long hundredths);

CFASSFileChangeRef CFASSFileChangeBlod(bool blod);

CFASSFileChangeRef CFASSFileChangeCombine(CFASSFileChangeRef change1, CFASSFileChangeRef change2, bool transferOwnershipIfCombinationSuccess);

CFASSFileChangeRef CFASSFileMultiChangeCombineTermiateWithNULL(bool transferOwnershipIfCombinationSuccess, CFASSFileChangeRef change1, ...);

void CFASSFileChangeDestory(CFASSFileChangeRef change);

#endif /* CFASSFileChange_h */
