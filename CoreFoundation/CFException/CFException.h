//
//  CFException.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/18.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFException_h
#define CFException_h

#include <stdbool.h>

#pragma mark - Contant excpetion name

extern const char * const CFExceptionNameArrayOutOfBounds;

extern const char * const CFExceptionNameDivideZero;

extern const char * const CFExceptionNameOverflow;

extern const char * const CFExceptionNameInvalidArgument;

extern const char * const CFExceptionNameProcessFailed;

#pragma mark - Function

typedef bool (*CFExceptionCatchFunction)(const char * exceptionName, void * data);

void CFExceptionRaise(const char * exceptionName, void * data, const char * format, ...)
__attribute__((format(printf, 3, 4)));

void CFExceptionCatchStackPush(CFExceptionCatchFunction function);

CFExceptionCatchFunction CFExceptionCatchStackPop(void);

CFExceptionCatchFunction CFExceptionCatchStackCurrentFunction(void);

#endif /* CFException_h */
