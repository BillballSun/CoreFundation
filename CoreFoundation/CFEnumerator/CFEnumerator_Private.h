//
//  CFEnumerator_Private.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/6.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFEnumerator_Private_h
#define CFEnumerator_Private_h

#include "CFEnumerator.h"

CFEnumeratorRef CFEnumeratorCreate(CFPointerArrayRef data, size_t *countCheckPoint, bool transferArrayOwnership);

CFEnumeratorRef CFEnumeratorCreateFromArray(CFPointerArrayRef array);

#endif /* CFEnumerator_Private_h */
