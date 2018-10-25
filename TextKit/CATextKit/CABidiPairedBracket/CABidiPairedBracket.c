//
//  CABidiPairedBracket.c
//  CoreFoundation
//
//  Created by Bill Sun on 2018/10/5.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CABidiPairedBracket.h"
#include "CFException.h"

typedef struct CABidiBracketPairs {
    CABidiPairedBracketType type;
    UTF32Char ch, opposite;
} CABidiBracketPairs;

static CABidiBracketPairs *CABidiPairDatabase = NULL;  /* if database loaded, this is not NULL */
static unsigned long databaseSize = 0lu;

static void CABidiPairedBracketLoadDefaultDatabase(void);
static void CABidiPairedBracketLoadDatabaseFromString(const char *string);
static void CABidiPairedBracketUnloadDatabase(void);
static CABidiBracketPairs *CABidiPairedBracketFindPairsForCharacter(UTF32Char ch);

CABidiPairedBracketType CABidiGetPairedBracketType(UTF32Char character, UTF32Char *pairedBracket)
{
    if(CABidiPairDatabase == NULL) CABidiPairedBracketLoadDefaultDatabase();
    if(CABidiPairDatabase != NULL)
    {
        CABidiBracketPairs *pairs = CABidiPairedBracketFindPairsForCharacter(character);
        if(pairs != NULL)
        {
            if(pairedBracket != NULL) *pairedBracket = pairs->opposite;
            return pairs->type;
        }
    }
    return CABidiPairedBracketTypeNone;
}

#pragma mark - Internal Database

static const char *default_BidiBracketDatabase =
"0028; 0029; o\n"
"0029; 0028; c\n"
"005B; 005D; o\n"
"005D; 005B; c\n"
"007B; 007D; o\n"
"007D; 007B; c\n"
"0F3A; 0F3B; o\n"
"0F3B; 0F3A; c\n"
"0F3C; 0F3D; o\n"
"0F3D; 0F3C; c\n"
"169B; 169C; o\n"
"169C; 169B; c\n"
"2045; 2046; o\n"
"2046; 2045; c\n"
"207D; 207E; o\n"
"207E; 207D; c\n"
"208D; 208E; o\n"
"208E; 208D; c\n"
"2308; 2309; o\n"
"2309; 2308; c\n"
"230A; 230B; o\n"
"230B; 230A; c\n"
"2329; 232A; o\n"
"232A; 2329; c\n"
"2768; 2769; o\n"
"2769; 2768; c\n"
"276A; 276B; o\n"
"276B; 276A; c\n"
"276C; 276D; o\n"
"276D; 276C; c\n"
"276E; 276F; o\n"
"276F; 276E; c\n"
"2770; 2771; o\n"
"2771; 2770; c\n"
"2772; 2773; o\n"
"2773; 2772; c\n"
"2774; 2775; o\n"
"2775; 2774; c\n"
"27C5; 27C6; o\n"
"27C6; 27C5; c\n"
"27E6; 27E7; o\n"
"27E7; 27E6; c\n"
"27E8; 27E9; o\n"
"27E9; 27E8; c\n"
"27EA; 27EB; o\n"
"27EB; 27EA; c\n"
"27EC; 27ED; o\n"
"27ED; 27EC; c\n"
"27EE; 27EF; o\n"
"27EF; 27EE; c\n"
"2983; 2984; o\n"
"2984; 2983; c\n"
"2985; 2986; o\n"
"2986; 2985; c\n"
"2987; 2988; o\n"
"2988; 2987; c\n"
"2989; 298A; o\n"
"298A; 2989; c\n"
"298B; 298C; o\n"
"298C; 298B; c\n"
"298D; 2990; o\n"
"298E; 298F; c\n"
"298F; 298E; o\n"
"2990; 298D; c\n"
"2991; 2992; o\n"
"2992; 2991; c\n"
"2993; 2994; o\n"
"2994; 2993; c\n"
"2995; 2996; o\n"
"2996; 2995; c\n"
"2997; 2998; o\n"
"2998; 2997; c\n"
"29D8; 29D9; o\n"
"29D9; 29D8; c\n"
"29DA; 29DB; o\n"
"29DB; 29DA; c\n"
"29FC; 29FD; o\n"
"29FD; 29FC; c\n"
"2E22; 2E23; o\n"
"2E23; 2E22; c\n"
"2E24; 2E25; o\n"
"2E25; 2E24; c\n"
"2E26; 2E27; o\n"
"2E27; 2E26; c\n"
"2E28; 2E29; o\n"
"2E29; 2E28; c\n"
"3008; 3009; o\n"
"3009; 3008; c\n"
"300A; 300B; o\n"
"300B; 300A; c\n"
"300C; 300D; o\n"
"300D; 300C; c\n"
"300E; 300F; o\n"
"300F; 300E; c\n"
"3010; 3011; o\n"
"3011; 3010; c\n"
"3014; 3015; o\n"
"3015; 3014; c\n"
"3016; 3017; o\n"
"3017; 3016; c\n"
"3018; 3019; o\n"
"3019; 3018; c\n"
"301A; 301B; o\n"
"301B; 301A; c\n"
"FE59; FE5A; o\n"
"FE5A; FE59; c\n"
"FE5B; FE5C; o\n"
"FE5C; FE5B; c\n"
"FE5D; FE5E; o\n"
"FE5E; FE5D; c\n"
"FF08; FF09; o\n"
"FF09; FF08; c\n"
"FF3B; FF3D; o\n"
"FF3D; FF3B; c\n"
"FF5B; FF5D; o\n"
"FF5D; FF5B; c\n"
"FF5F; FF60; o\n"
"FF60; FF5F; c\n"
"FF62; FF63; o\n"
"FF63; FF62; c\n";

#define DatabaseInitializedSize 1232
#define DatabaseIncreaseSize 250
#define MaxBidiPairTypeLength 1

#define Stringlization(x) _Stringlization(x)
#define _Stringlization(x) #x

#define LOOP for(;;)

static unsigned long databaseAllocatedSize = 0lu;

static void CABidiPairedBracketAddPairsFromString(const char *rangeString);
static CABidiPairedBracketType CABidiPairedBracketTypeFromString(const char *string);
static void CABidiPairedBracketInsertRangeIntoDatabase(CABidiBracketPairs range);
static int CABidiPairedBracketCompareCharacter(UTF32Char ch, CABidiBracketPairs pairs);

static CABidiBracketPairs *CABidiPairedBracketFindPairsForCharacter(UTF32Char ch)
{
    if(CABidiPairDatabase == NULL)
    {
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABidiPairedBucket database is not loaded");
        return NULL;
    }
    long beginIndex = 0;
    long endIndex = databaseSize - 1;
    do
    {
        long midIndex = (beginIndex + endIndex) / 2;
        int compare = CABidiPairedBracketCompareCharacter(ch, CABidiPairDatabase[midIndex]);
        if(compare < 0)
            endIndex = midIndex - 1;
        else if(compare == 0)
            return CABidiPairDatabase + midIndex;
        else
            beginIndex = midIndex + 1;
    }while(beginIndex <= endIndex);
    return NULL;
}

static int CABidiPairedBracketCompareCharacter(UTF32Char ch, CABidiBracketPairs pairs)
{
    if(ch < pairs.ch) return -1;
    if(ch > pairs.ch) return 1;
    return 0;
}

static void CABidiPairedBracketLoadDefaultDatabase(void)
{
    CABidiPairedBracketLoadDatabaseFromString(default_BidiBracketDatabase);
}

static void CABidiPairedBracketLoadDatabaseFromString(const char *string)
{
    const char *current = string;
    do
    {
        CABidiPairedBracketAddPairsFromString(current);
        do current++; while(*current!='\n' && *current!='\0');
        if(*current == '\n') current++;
        else break;
    }while (1);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
static void CABidiPairedBracketUnloadDatabase(void)
{
    if(CABidiPairDatabase != NULL)
    {
        free(CABidiPairDatabase);
        CABidiPairDatabase = NULL;
        databaseAllocatedSize = 0u;
        databaseSize = 0u;
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"

static void CABidiPairedBracketAddPairsFromString(const char *rangeString)
{
    UTF32Char ch, opposite;
    char typeString[MaxBidiPairTypeLength + 1];
    CABidiPairedBracketType type;
    bool valid = false;
    if(sscanf(rangeString, "%"UTF32CharSCNx"; "UTF32CharSCNx"; %" Stringlization(MaxBidiPairTypeLength) "s", &ch, &opposite, typeString) == 3)
        valid = true;
    if(valid)
    {
        type = CABidiPairedBracketTypeFromString(typeString);
        CABidiBracketPairs pairs = (CABidiBracketPairs){.type = type, .ch = ch, .opposite = opposite};
        
        CABidiPairedBracketInsertRangeIntoDatabase(pairs);
    }
}

#pragma clang diagnostic pop

static void CABidiPairedBracketInsertRangeIntoDatabase(CABidiBracketPairs pairs)
{
    if(CABidiPairDatabase == NULL)
    {
        if((CABidiPairDatabase = malloc(sizeof(CABidiBracketPairs) * DatabaseInitializedSize)) != NULL)
        {
            databaseAllocatedSize = DatabaseInitializedSize;
            databaseSize = 0;
        }
    }
    if(CABidiPairDatabase != NULL)
    {
        unsigned long insertIndex = 0;
        LOOP
        {
            if(insertIndex == databaseSize)
                break;
            if(pairs.ch == CABidiPairDatabase[insertIndex].ch)
            {
                CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABidiPairDatabase pairs exist");
                return;
            }
            else if(pairs.ch > CABidiPairDatabase[insertIndex].ch)
                insertIndex++;
            else
                break;
        }
        if(databaseSize >= databaseAllocatedSize)
        {
            CABidiBracketPairs *temp = realloc(CABidiPairDatabase, sizeof(CABidiBracketPairs) * (databaseAllocatedSize + DatabaseIncreaseSize));
            if(temp != NULL)
            {
                CABidiPairDatabase = temp;
                databaseAllocatedSize = databaseAllocatedSize + DatabaseIncreaseSize;
            }
            else
            {
                CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABidiPairedBracketInsertRangeIntoDatabase data size increased failed");
                return;
            }
        }
        for(size_t index = databaseSize; index > insertIndex; index--)
            CABidiPairDatabase[index] = CABidiPairDatabase[index - 1];
        
        CABidiPairDatabase[insertIndex] = pairs;
        databaseSize++;
    }
}

static CABidiPairedBracketType CABidiPairedBracketTypeFromString(const char *string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABidiPairedBracketTypeFromString NULL");
        return CABidiPairedBracketTypeNone;
    }
    
    if(strcmp(string, "o") == 0)
        return CABidiPairedBracketTypeOpen;
    else if(strcmp(string, "c") == 0)
        return CABidiPairedBracketTypeClose;
    else if(strcmp(string, "n") == 0)
        return CABidiPairedBracketTypeNone;
    else
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABidiPairedBracketTypeFromString invalid \"%s\"", string);
        return CABidiPairedBracketTypeNone;
    }
}

