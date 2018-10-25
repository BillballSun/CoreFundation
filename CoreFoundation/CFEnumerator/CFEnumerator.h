//
//  CFEnumerator.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/6/6.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFEnumerator_h
#define CFEnumerator_h

#include "CFPointerArray.h"

typedef struct CFEnumerator *CFEnumeratorRef;

void *CFEnumeratorNextObject(CFEnumeratorRef enumerator);

CFPointerArrayRef CFEnumeratorGetRemainedObjects(CFEnumeratorRef enumerator);

void CFEnumeratorDestory(CFEnumeratorRef enumerator);

CFEnumeratorRef CFEnumeratorCreateFromArray(CFPointerArrayRef array);

#endif /* CFEnumerator_h */
