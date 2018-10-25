//
//  CFASSFileDialogueCollection.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueCollection_h
#define CFASSFileDialogueCollection_h

typedef struct CFASSFileDialogueCollection *CFASSFileDialogueCollectionRef;

#pragma mark - Copy/Destory

CFASSFileDialogueCollectionRef CFASSFileDialogueCollectionCopy(CFASSFileDialogueCollectionRef dialogueCollection);

void CFASSFileDialogueCollectionDestory(CFASSFileDialogueCollectionRef dialogueCollection);

#pragma mark - Get Component

CFEnumeratorRef CFASSFileDialogueCollectionCreateEnumerator(CFASSFileDialogueCollectionRef dialogueCollection);

#pragma mark - Receive Change

void CFASSFileDialogueCollectionMakeChange(CFASSFileDialogueCollectionRef dialogueCollection, CFASSFileChangeRef change);

#endif /* CFASSFileDialogueCollection_h */
