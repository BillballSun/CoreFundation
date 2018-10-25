//
//  main.m
//  TextKit
//
//  Created by Bill Sun on 2018/10/1.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include "CATextKit/UTF32String/UTF32String.h"
#include "CATextKit/CAPunycode/CAPunycode.h"

int main(int argc, const char * argv[]) {
    
    UTF32StringRef result = CAPunycodeEecodeUnicodeString((UTF32Char []){0x5E05, 0x54E5, 0});
    
    for(size_t index = 0; index < UTF32StringGetLength(result); index++)
    {
        UTF32Char ch = UTF32StringGetCharAtIndex(result, index);
        fprintf(stdout, "Index %zu: %c\n", index, ch);
    }
    
    return 0;
}
