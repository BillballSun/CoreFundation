//
//  CFASSFileStyleCollection.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileStyleCollection_h
#define CFASSFileStyleCollection_h

#include "CFEnumerator.h"

typedef struct CFASSFileStyleCollection *CFASSFileStyleCollectionRef;

#pragma mark - Create/Copy/Destory

CFASSFileStyleCollectionRef CFASSFileStyleCollectionCopy(CFASSFileStyleCollectionRef styleCollection);

void CFASSFileStyleCollectionDestory(CFASSFileStyleCollectionRef styleCollection);

#pragma mark - Get Component

CFEnumeratorRef CFASSFileStyleCollectionCreateEnumerator(CFASSFileStyleCollectionRef styleCollection);

#pragma mark - Receive Change

void CFASSFileStyleCollectionMakeChange(CFASSFileStyleCollectionRef styleCollection, CFASSFileChangeRef change);

#endif /* CFASSFileStyleCollection_h */
