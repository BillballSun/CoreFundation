//
//  CABLGenerator.c
//  TextSystem
//
//  Created by Bill Sun on 2018/9/30.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#pragma mark - Header inclusion

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "CABLGenerator.h"
#include "CABidiClassify.h"
#include "CFException.h"
#include "CABidiPairedBracket.h"

#pragma mark - Macro configuration

#define max_depth 125
#define TryToPairBracketStackSize 63

#pragma mark - Macro simplification

#define LeastOddGreaterThan(x) ((x) + ((x) % 2 == 0 ? 1 : 2))
#define LeastEvenGreaterThan(x) ((x) + ((x) % 2 == 0 ? 2 : 1))
#define LOOP for(;;)
#define EmbeddingDirection(level) ((level) % 2 == 0 ? CABidiTypeL : CABidiTypeR)

#pragma mark - Type definition

struct CABLGenerator {
    size_t length;          // this could not be zero, string length zero will failed to create CABLGenerator
    UTF32StringRef string;
    CABidiLevel *levelArr;
    CABidiType *typeArr;
};

typedef struct CAMatchingExplictFormat *CAMatchingExplictFormatRef;
typedef struct CAIsolatingRunSequencesData *CAIsolatingRunSequencesDataRef;

#pragma mark - Static function declearation

/* process bidi-level */
static void CABLGeneratorProcessBidiLevel(CABLGeneratorRef generator);  // will consider CRCL status as one paragraph
static void CABLGeneratorProcessPragraphLevel(CABLGeneratorRef generator, size_t paragraphBeginIndex, size_t paragraphEndIndex);
static CABidiLevel CABLGeneratorResolvePragraphEmbeddingLevel(CABLGeneratorRef generator, size_t paragraphBeginIndex, size_t paragraphEndIndex);

/* matchingExplictFormat */
static CAMatchingExplictFormatRef CABLGeneratorResolveMatchingExplictFormat(CABLGeneratorRef generator, size_t paragraphBeginIndex, size_t paragraphEndIndex);
static void CAMatchingExplictFormatDestory(CAMatchingExplictFormatRef matchingExplictFormat);
static bool CAMatchingExplictFormatLocateMatchingPDI(CAMatchingExplictFormatRef matchingExplictFormat, size_t isolateInitializer, size_t *matchingPDIIndex);
static bool CAMatchingExplictFormatLocateMatchingPDF(CAMatchingExplictFormatRef matchingExplictFormat, size_t embeddingInitializer, size_t *matchingPDFIndex);
static bool CAMatchingExplictFormatLocateMatchingIsolateInitializer(CAMatchingExplictFormatRef matchingExplictFormat, size_t PDI_Index, size_t *isolateIntializerIndex);

/* isolate run sequences data */
static CAIsolatingRunSequencesDataRef CAIsolatingRunSequencesDataCreate(CABLGeneratorRef generator, CAMatchingExplictFormatRef matching, size_t paragraphBeginIndex, size_t paragrapEndIndex);
static void CAIsolatingRunSequencesDataDestory(CAIsolatingRunSequencesDataRef data);
static size_t CAIsolatingRunSequencesDataGetCharacterIndex(CAIsolatingRunSequencesDataRef data, size_t sequenceArrIndex, size_t characterIndexInThisSequence);
static size_t CAIsolatingRunSequencesDataGetSequenceAmount(CAIsolatingRunSequencesDataRef data);
static size_t CAIsolatingRunSequencesDataGetCharacterAmountInSequence(CAIsolatingRunSequencesDataRef data, size_t sequenceArrIndex);

#pragma mark - CABLGenerator (ADT)

CABLGeneratorRef CABLGeneratorCreateWithString(UTF32StringRef string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABLGeneratorCreateWithString NULL");
        return NULL;
    }
    if(UTF32StringGetLength(string) == 0)
        return NULL;
    UTF32StringRef copyedString;
    if((copyedString = UTF32StringCopy(string)) == NULL)
    {
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorCreateWithString copy string failed");
        return NULL;
    }
    CABLGeneratorRef result;
    if((result = malloc(sizeof(struct CABLGenerator))) != NULL)
    {
        size_t length = UTF32StringGetLength(copyedString);
        if((result->levelArr = malloc(sizeof(CABidiLevel) * length)) != NULL)
        {
            if((result->typeArr = malloc(sizeof(CABidiType) * length)) != NULL)
            {
                result->length = length;
                result->string = copyedString;
                CABLGeneratorProcessBidiLevel(result);
                return result;
            }
            free(result->levelArr);
        }
        free(result);
    }
    UTF32StringDestory(copyedString);
    return NULL;
}

static void CABLGeneratorProcessBidiLevel(CABLGeneratorRef generator)
{
    for(size_t index = 0; index < generator->length; index++)
        generator->typeArr[index] = CABidiClassifyTypeForCharacter(UTF32StringGetCharAtIndex(generator->string, index));
    //    CR \r 0x0D
    //    LF \n 0x0A
    size_t beginIndex = 0, endIndex;
    size_t length = generator->length;
    do
    {
        endIndex = beginIndex;
        
        while(generator->typeArr[endIndex] != CABidiTypeB && endIndex < length - 1) endIndex++;
        
        /* handle CRLF */
        if(endIndex < length - 1
           && UTF32StringGetCharAtIndex(generator->string, endIndex) == '\r'
           && UTF32StringGetCharAtIndex(generator->string, endIndex + 1) == '\n')
            endIndex++;
        
        /* P1 - Unicode Bidirectional Algorithm */
        
        CABLGeneratorProcessPragraphLevel(generator, beginIndex, endIndex);
        
        beginIndex = endIndex + 1;
    }while(beginIndex < length);
}

#pragma mark Main process function (CABLGeneratorProcessPragraphLevel)

typedef enum CADirectionalOverrideStatus {
    CADirectionalOverrideStatusNeutral,
    CADirectionalOverrideStatusLeftToRight,
    CADirectionalOverrideStatusRightToLeft,
} CADirectionalOverrideStatus;

typedef bool CADirectionalIsolateStatus;

typedef struct directionalStatusElement {
    CABidiLevel level;
    CADirectionalOverrideStatus overrideStatus;
    CADirectionalIsolateStatus isolateStatus;
} directionalStatusElement;

typedef struct tryToPairElement {
    UTF32Char pairedCloseCharacter;
    size_t index;
} tryToPairElement;

typedef struct pairedBracket {
    size_t openIndex;
    size_t closeIndex;
} pairedBracket;

static void CABLGeneratorProcessPragraphLevel(CABLGeneratorRef generator, size_t beginIndex, size_t endIndex)
{
    CAMatchingExplictFormatRef matchingExplictFormat;
    if((matchingExplictFormat = CABLGeneratorResolveMatchingExplictFormat(generator, beginIndex, endIndex)) != NULL)
    {
        /* P2 P3 - Unicode Bidirectional Algorithm */
        /* resolve paragraph level */
        
        CABidiLevel paragraphLevel = 0;
        for(size_t index = beginIndex; index <= endIndex; index++)
        {
            CABidiType currentType = generator->typeArr[index];
            if(currentType == CABidiTypeAL || currentType == CABidiTypeR)
                { paragraphLevel = 1; break; }
            else if(isExplicitDirectionalIsolates(currentType))
            {
                size_t matchingPDIIndex;
                if(CAMatchingExplictFormatLocateMatchingPDI(matchingExplictFormat, index, &matchingPDIIndex))
                    index = matchingPDIIndex;   // not plus 1, consider about it
                else
                    index = endIndex;
            }
        }
        
        /* X1 - Unicode Bidirectional Algorithm */
        /* directional status stack */
        
        directionalStatusElement statusStack[max_depth + 2];
        size_t topIndex = 0u;
        statusStack[topIndex] = (directionalStatusElement){.level = paragraphLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = false};
        
        size_t overflowIsolateCount = 0u;
        // This reflects the number of isolate initiators that were encountered in the pass so far without encountering their matching PDIs, but were invalidated by the depth limit and thus are not reflected in the directional status stack.
        size_t overflowEmbeddingCount = 0u;
        // This reflects the number of embedding initiators that were encountered in the pass so far without encountering their matching PDF, or encountering the PDI of an isolate within which they are nested, but were invalidated by the depth limit, and thus are not reflected in the directional status stack.
        size_t validIsolateCount = 0u;
        // This reflects the number of isolate initiators that were encountered in the pass so far without encountering their matching PDIs, and have been judged valid by the depth limit, i.e. all the entries on the stack with a true directional isolate status.
        
        CABidiLevel currentLevel = 0;
        for(size_t index = beginIndex; index < endIndex; index++)
        {
            // Only embedding levels from 0 through max_depth are valid in this phase.
            
            CABidiType currentType = generator->typeArr[index];
            
            /* X2 - Unicode Bidirectional Algorithm */
            if(currentType == CABidiTypeRLE)        // Level ignored for RLE
            {
                currentLevel = statusStack[topIndex].level;
                
                // Compute the least odd embedding level greater than the embedding level of the last entry on the directional status stack.
                CABidiLevel tryLevel = LeastOddGreaterThan(statusStack[topIndex].level);
                if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                {
                    // this RLE is valid
                    // Push an entry consisting of the new embedding level, neutral directional override status, and false directional isolate status onto the directional status stack.
                    statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = false};
                }
                else
                {
                    // this is an overflow LRE
                    // If the overflow isolate count is zero, increment the overflow embedding count by one.
                    if(overflowIsolateCount == 0) overflowEmbeddingCount++;
                }
            }
            /* X3 - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeLRE)   // Level ignored for LRE
            {
                currentLevel = statusStack[topIndex].level;
                
                // Compute the least even embedding level greater than the embedding level of the last entry on the directional status stack.
                CABidiLevel tryLevel = LeastEvenGreaterThan(statusStack[topIndex].level);
                if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                {
                    // this LRE is valid
                    // Push an entry consisting of the new embedding level, neutral directional override status, and false directional isolate status onto the directional status stack.
                    statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = false};
                }
                else
                {
                    // this is an overflow LRE
                    // If the overflow isolate count is zero, increment the overflow embedding count by one.
                    if(overflowIsolateCount == 0) overflowEmbeddingCount++;
                }
            }
            /* X4 - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeRLO)   // Level ignored for RLO
            {
                currentLevel = statusStack[topIndex].level;
                
                // Compute the least odd embedding level greater than the embedding level of the last entry on the directional status stack.
                CABidiLevel tryLevel = LeastOddGreaterThan(statusStack[topIndex].level);
                if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                {
                    // this RLE is valid
                    // Push an entry consisting of the new embedding level, neutral directional override status, and false directional isolate status onto the directional status stack.
                    statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusRightToLeft, .isolateStatus = false};
                }
                else
                {
                    // this is an overflow LRE
                    // If the overflow isolate count is zero, increment the overflow embedding count by one.
                    if(overflowIsolateCount == 0) overflowEmbeddingCount++;
                }
            }
            /* X5 - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeLRO)   // Level ignored for LRO
            {
                currentLevel = statusStack[topIndex].level;
                
                // Compute the least even embedding level greater than the embedding level of the last entry on the directional status stack.
                CABidiLevel tryLevel = LeastEvenGreaterThan(statusStack[topIndex].level);
                if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                {
                    // this LRE is valid
                    // Push an entry consisting of the new embedding level, neutral directional override status, and false directional isolate status onto the directional status stack.
                    statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusLeftToRight, .isolateStatus = false};
                }
                else
                {
                    // this is an overflow LRE
                    // If the overflow isolate count is zero, increment the overflow embedding count by one.
                    if(overflowIsolateCount == 0) overflowEmbeddingCount++;
                }
            }
            /* X5a - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeRLI)
            {
                // Set the RLI’s embedding level to the embedding level of the last entry on the directional status stack.
                currentLevel = statusStack[topIndex].level;
                
                if(statusStack[topIndex].overrideStatus != CADirectionalOverrideStatusNeutral)
                {
                    if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusLeftToRight)
                        generator->typeArr[index] = CABidiTypeL;
                    else if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusRightToLeft)
                        generator->typeArr[index] = CABidiTypeR;
                }
                
                CABidiLevel tryLevel = LeastOddGreaterThan(statusStack[topIndex].level);
                if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                {
                    validIsolateCount++;
                    statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = true};
                }
                else
                    overflowIsolateCount++;
            }
            /* X5b - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeLRI)
            {
                // Set the LRI’s embedding level to the embedding level of the last entry on the directional status stack.
                currentLevel = statusStack[topIndex].level;
                
                if(statusStack[topIndex].overrideStatus != CADirectionalOverrideStatusNeutral)
                {
                    if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusLeftToRight)
                        generator->typeArr[index] = CABidiTypeL;
                    else if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusRightToLeft)
                        generator->typeArr[index] = CABidiTypeR;
                }
                
                CABidiLevel tryLevel = LeastEvenGreaterThan(statusStack[topIndex].level);
                if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                {
                    validIsolateCount++;
                    statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = true};
                }
                else
                    overflowIsolateCount++;
            }
            /* X5c - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeFSI)
            {
                size_t matchingPDIIndex;
                if(!CAMatchingExplictFormatLocateMatchingPDI(matchingExplictFormat, index, &matchingPDIIndex))
                    matchingPDIIndex = endIndex;
                
                CABidiLevel FSILevel = 0;
                for(size_t i = index; i <= matchingPDIIndex; i++)
                {
                    CABidiType currentType = generator->typeArr[i];
                    if(currentType == CABidiTypeAL || currentType == CABidiTypeR)
                    { FSILevel = 1; break; }
                    else if(isExplicitDirectionalIsolates(currentType))
                    {
                        size_t internalMatchingPDIIndex;
                        if(CAMatchingExplictFormatLocateMatchingPDI(matchingExplictFormat, i, &internalMatchingPDIIndex))
                            i = internalMatchingPDIIndex;   // not plus 1, consider about it
                        else
                            i = endIndex;
                    }
                }
                if(FSILevel == 1)
                {
                    // treat as RLI
                    
                    // Set the RLI’s embedding level to the embedding level of the last entry on the directional status stack.
                    currentLevel = statusStack[topIndex].level;
                    
                    if(statusStack[topIndex].overrideStatus != CADirectionalOverrideStatusNeutral)
                    {
                        if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusLeftToRight)
                            generator->typeArr[index] = CABidiTypeL;
                        else if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusRightToLeft)
                            generator->typeArr[index] = CABidiTypeR;
                    }
                    
                    CABidiLevel tryLevel = LeastOddGreaterThan(statusStack[topIndex].level);
                    if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                    {
                        validIsolateCount++;
                        statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = true};
                    }
                    else
                        overflowIsolateCount++;
                }
                else
                {
                    // treat as LRI
                    
                    // Set the LRI’s embedding level to the embedding level of the last entry on the directional status stack.
                    currentLevel = statusStack[topIndex].level;
                    
                    if(statusStack[topIndex].overrideStatus != CADirectionalOverrideStatusNeutral)
                    {
                        if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusLeftToRight)
                            generator->typeArr[index] = CABidiTypeL;
                        else if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusRightToLeft)
                            generator->typeArr[index] = CABidiTypeR;
                    }
                    
                    CABidiLevel tryLevel = LeastEvenGreaterThan(statusStack[topIndex].level);
                    if(tryLevel <= max_depth && overflowIsolateCount == 0 && overflowEmbeddingCount == 0)
                    {
                        validIsolateCount++;
                        statusStack[++topIndex] = (directionalStatusElement){.level = tryLevel, .overrideStatus = CADirectionalOverrideStatusNeutral, .isolateStatus = true};
                    }
                    else
                        overflowIsolateCount++;
                }
            }
            /* X6a - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypePDI)
            {
                if(overflowIsolateCount > 0) overflowIsolateCount--;
                else if(validIsolateCount != 0)
                {
                    overflowEmbeddingCount = 0;
                    while(statusStack[topIndex].isolateStatus == false) topIndex--;
                    
                    topIndex--;
                    validIsolateCount--;
                }
                
                currentLevel = statusStack[topIndex].level;
                
                if(statusStack[topIndex].overrideStatus != CADirectionalOverrideStatusNeutral)
                {
                    if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusLeftToRight)
                        generator->typeArr[index] = CABidiTypeL;
                    else if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusRightToLeft)
                        generator->typeArr[index] = CABidiTypeR;
                }
            }
            /* X7 - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypePDF)   // Level ignored for PDF
            {
                if(overflowIsolateCount == 0)
                {
                    if(overflowEmbeddingCount >0) overflowEmbeddingCount--;
                    else if(!statusStack[topIndex].isolateStatus && topIndex >= 1)
                    {
                        topIndex--;
                        currentLevel = statusStack[topIndex].level;
                    }
                }
                currentLevel = statusStack[topIndex].level;
            }
            /* X8 - Unicode Bidirectional Algorithm */
            else if(currentType == CABidiTypeB)
            {
                currentLevel = paragraphLevel;
            }
            /* X6 - Unicode Bidirectional Algorithm */
            else
            {
                currentLevel = statusStack[topIndex].level;
                if(statusStack[topIndex].overrideStatus != CADirectionalOverrideStatusNeutral)
                {
                    if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusLeftToRight)
                        generator->typeArr[index] = CABidiTypeL;
                    else if(statusStack[topIndex].overrideStatus == CADirectionalOverrideStatusRightToLeft)
                        generator->typeArr[index] = CABidiTypeR;
                }
            }
            generator->levelArr[index] = currentLevel;
        }
        
        /* X9 - Unicode Bidirectional Algorithm */
        /* Preparations for Implicit Processing */
        for(size_t index = beginIndex; index <= endIndex; index++)
        {
            CABidiType currentType = generator->typeArr[index];
            
            if(isExplicitDirectionalEmbeddings(currentType) || currentType == CABidiTypePDF)
                generator->typeArr[index] = CABidiTypeBN;
        }
        
        /* X10 - Unicode Bidirectional Algorithm */
        /* divided into Isolate Run Sequence */
        CAIsolatingRunSequencesDataRef sequenceData;
        if((sequenceData = CAIsolatingRunSequencesDataCreate(generator, matchingExplictFormat, beginIndex, endIndex)) != NULL)
        {
            size_t sequencesAmount = CAIsolatingRunSequencesDataGetSequenceAmount(sequenceData);
            for(size_t sequenceIndex = 0; sequenceIndex < sequencesAmount; sequenceIndex++)
            {
                size_t characterAmountInSequence = CAIsolatingRunSequencesDataGetCharacterAmountInSequence(sequenceData, sequenceIndex);
                
                size_t firstCharacterInSequenceParagraphIndex = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, 0);
                size_t lastCharacterInSequenceParagraphIndex = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterAmountInSequence - 1);
                
                
                CABidiLevel firstLevel = generator->levelArr[firstCharacterInSequenceParagraphIndex];
                CABidiLevel lastLevel = generator->levelArr[lastCharacterInSequenceParagraphIndex];
                
                CABidiLevel formerLevel;    /* skip over any BNs */
                size_t formerIndex = firstCharacterInSequenceParagraphIndex - 1;
                while (generator->typeArr[formerIndex] == CABidiTypeBN && formerIndex >= beginIndex)
                    formerIndex--;
                if(formerIndex>=beginIndex) formerLevel = generator->levelArr[formerIndex];
                else formerLevel = paragraphLevel;
                
                CABidiLevel latterLevel;    /* skip over any BNs */
                size_t latterIndex = lastCharacterInSequenceParagraphIndex + 1;
                while (generator->typeArr[latterIndex] == CABidiTypeBN && latterIndex <= endIndex)
                    latterIndex++;
                if(latterIndex <= endIndex) latterLevel = generator->levelArr[latterIndex];
                else latterLevel = paragraphLevel;
                
                /* resolve SOS & EOS */
                CABidiType sos = (firstLevel > formerLevel ? firstLevel : formerLevel) % 2 == 0 ? CABidiTypeL : CABidiTypeR;
                CABidiType eos = (lastLevel > latterLevel ? lastLevel : latterLevel) % 2 == 0 ? CABidiTypeL : CABidiTypeR;
                
                /* Resolving Weak Types */
                
                /* W1 - Unicode Bidirectional Algorithm */
                /* caculate non-space mark amount */
                size_t nonSpaceMarkAmount = 0;
                size_t *nonSpaceMarkIndexInSequenceArr = NULL;
                /* In this array, first is the NSM index, then it is the previous matching non-BN index in sequence, if it matches sos, it is SIZE_MAX */
                
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeNSM)
                        nonSpaceMarkAmount++;
                }
                
                /* resolve non-space mark */
                if(nonSpaceMarkAmount > 0)
                {
                    if((nonSpaceMarkIndexInSequenceArr = malloc(sizeof(size_t) * nonSpaceMarkAmount * 2)) != NULL)
                    {
                        size_t nonSpaceMarkArrIndex = 0;
                        for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                        {
                            size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                            
                            CABidiType currentType = generator->typeArr[currentIndexInString];
                            if(currentType == CABidiTypeNSM)
                            {
                                nonSpaceMarkIndexInSequenceArr[nonSpaceMarkArrIndex++] = characterIndexInSequence;
                                bool isFirstNonBNcharacter = true;
                                for(long formerIndexInSequence = characterIndexInSequence; formerIndexInSequence >= 0; formerIndexInSequence--)
                                {
                                    size_t formerIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, formerIndexInSequence);
                                    CABidiType formerType = generator->typeArr[formerIndexInString];
                                    if(formerType != CABidiTypeBN)
                                    {
                                        isFirstNonBNcharacter = false;
                                        nonSpaceMarkIndexInSequenceArr[nonSpaceMarkArrIndex++] = formerIndexInSequence;
                                        
                                        if(isExplicitDirectionalIsolates(formerType) || formerType == CABidiTypePDI)
                                            generator->typeArr[currentIndexInString] = CABidiTypeON;
                                        else
                                            generator->typeArr[currentIndexInString] = formerType;
                                        break;
                                    }
                                }
                                
                                if(isFirstNonBNcharacter)
                                {
                                    nonSpaceMarkIndexInSequenceArr[nonSpaceMarkArrIndex++] = SIZE_MAX;
                                    generator->typeArr[currentIndexInString] = sos;
                                }
                            }
                        }
                    }
                    else
                    {
                        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorProcessPragraphLevel allocate pairsInSequence failed");
                        CAIsolatingRunSequencesDataDestory(sequenceData);
                        CAMatchingExplictFormatDestory(matchingExplictFormat);
                        return;
                    }
                }
                
                /* W2 - Unicode Bidirectional Algorithm */
                /* resolve European number to AN based on mostly closed strong type */
                for(long characterIndexInSequence = characterAmountInSequence - 1; characterIndexInSequence >= 0; characterIndexInSequence--)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeEN)
                    {
                        long backwardIndexInSequence;
                        for(backwardIndexInSequence = characterIndexInSequence - 1; backwardIndexInSequence >= 0; backwardIndexInSequence--)
                        {
                            size_t backwardIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, backwardIndexInSequence);
                            CABidiType backwardType = generator->typeArr[backwardIndexInString];
                            if(CABidiClassifyCategoryForType(backwardType) == CABidiCategoryStrong)
                            {
                                if(backwardType == CABidiTypeAL)
                                    generator->typeArr[currentIndexInString] = CABidiTypeAN;
                                break;
                            }
                        }
                        /* break exit */
                        if(backwardIndexInSequence < 0) /* check sos */
                            if(sos == CABidiTypeAL)
                                generator->typeArr[currentIndexInString] = CABidiTypeAN;
                    }
                }
                
                /* W3 - Unicode Bidirectional Algorithm */
                /* change all AL to R */
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeAL)
                        generator->typeArr[currentIndexInString] = CABidiTypeR;
                }
                
                /* W4 - Unicode Bidirectional Algorithm */
                /* A single European separator between two European numbers changes to a European number */
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeES)
                    {
                        CABidiType previousType = CABidiTypeBN; /* just avoid un-initialization warning, useless assignment */
                        bool isFirstNonBN = true;
                        for(long formerIndexInSequence = characterIndexInSequence; formerIndexInSequence >= 0; formerIndexInSequence--)
                        {
                            size_t formerIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, formerIndexInSequence);
                            previousType = generator->typeArr[formerIndexInString];
                            if(previousType != CABidiTypeBN)
                            {
                                isFirstNonBN = false;
                                break;
                            }
                        }
                        if(isFirstNonBN) previousType = sos;
                        
                        CABidiType latterType = CABidiTypeBN; /* just avoid un-initialization warning, useless assignment */
                        bool isLastNonBN = true;
                        for(long latterIndexInSequence = characterIndexInSequence; latterIndexInSequence < characterAmountInSequence; latterIndexInSequence--)
                        {
                            size_t latterIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, latterIndexInSequence);
                            latterType = generator->typeArr[latterIndexInString];
                            if(latterType != CABidiTypeBN)
                            {
                                isLastNonBN = false;
                                break;
                            }
                        }
                        if(isLastNonBN) latterIndex = eos;
                        
                        if(previousType == CABidiTypeEN && latterType == CABidiTypeEN)
                            generator->typeArr[currentIndexInString] = CABidiTypeEN;
                    }
                }
                
                /* A single common separator between two numbers of the same type changes to that type */
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeCS)
                    {
                        CABidiType previousType = CABidiTypeBN; /* just avoid un-initialization warning, useless assignment */
                        bool isFirstNonBN = true;
                        for(long formerIndexInSequence = characterIndexInSequence; formerIndexInSequence >= 0; formerIndexInSequence--)
                        {
                            size_t formerIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, formerIndexInSequence);
                            previousType = generator->typeArr[formerIndexInString];
                            if(previousType != CABidiTypeBN)
                            {
                                isFirstNonBN = false;
                                break;
                            }
                        }
                        if(isFirstNonBN) previousType = sos;
                        
                        CABidiType latterType = CABidiTypeBN; /* just avoid un-initialization warning, useless assignment */
                        bool isLastNonBN = true;
                        for(long latterIndexInSequence = characterIndexInSequence; latterIndexInSequence < characterAmountInSequence; latterIndexInSequence--)
                        {
                            size_t latterIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, latterIndexInSequence);
                            latterType = generator->typeArr[latterIndexInString];
                            if(latterType != CABidiTypeBN)
                            {
                                isLastNonBN = false;
                                break;
                            }
                        }
                        if(isLastNonBN) latterIndex = eos;
                        
                        if(previousType == CABidiTypeEN && latterType == CABidiTypeEN)
                            generator->typeArr[currentIndexInString] = CABidiTypeEN;
                        
                        if(previousType == latterType && (previousType == CABidiTypeEN || previousType == CABidiTypeAN))
                            generator->typeArr[currentIndexInString] = previousType;
                    }
                }
                
                /* W5 - Unicode Bidirectional Algorithm */
                /* A sequence of European terminators adjacent to European numbers changes to all European numbers */
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeET || currentType == CABidiTypeBN)
                    {
                        size_t terminatorsBeginIndexInSequence = characterIndexInSequence;
                        size_t terminatorsEndIndexInSequence = terminatorsBeginIndexInSequence;
                        
                        LOOP
                        {
                            if(terminatorsEndIndexInSequence + 1 >= characterAmountInSequence)
                                break;
                            CABidiType terminatorsEndType = generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, terminatorsEndIndexInSequence + 1)];
                            if(terminatorsEndType == CABidiTypeET || terminatorsEndType == CABidiTypeBN)
                                terminatorsEndType++;
                            else
                                break;
                        }
                        
                        CABidiType previousType;
                        if(terminatorsBeginIndexInSequence == 0) previousType = sos;
                        else previousType = generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, terminatorsBeginIndexInSequence - 1)];
                        
                        CABidiType latterType;
                        if(terminatorsEndIndexInSequence == characterAmountInSequence - 1) latterType = eos;
                        else latterType = generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, terminatorsEndIndexInSequence + 1)];
                        
                        if(previousType == CABidiTypeEN || previousType == CABidiTypeEN)
                            for(size_t currentTerminatorIndexInSequence = terminatorsBeginIndexInSequence; currentTerminatorIndexInSequence <= terminatorsEndIndexInSequence; currentTerminatorIndexInSequence++)
                                generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, currentTerminatorIndexInSequence)] = CABidiTypeEN;
                        
                        characterIndexInSequence = terminatorsEndIndexInSequence;
                    }
                }
                
                /* W6 - Unicode Bidirectional Algorithm */
                /* Otherwise, separators and terminators (only weak type) change to Other Neutral */
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeES || currentType == CABidiTypeET || currentType == CABidiTypeCS)
                    {
                        if(characterIndexInSequence > 0)
                        {
                            size_t formerIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence - 1);
                            CABidiType formerType = generator->typeArr[formerIndexInString];
                            if(formerType == CABidiTypeBN)
                                generator->typeArr[formerIndexInString] = CABidiTypeON;
                        }
                        if(characterIndexInSequence < characterAmountInSequence - 1)
                        {
                            size_t latterIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence + 1);
                            CABidiType latterType = generator->typeArr[latterIndexInString];
                            if(latterType == CABidiTypeBN)
                                generator->typeArr[latterIndexInString] = CABidiTypeON;
                        }
                        generator->typeArr[currentIndexInString] = CABidiTypeON;
                    }
                }
                
                /* W7 - Unicode Bidirectional Algorithm */
                /* Search backward from each instance of a European number until the first strong type (R, L, or sos) is found */
                /* If an L is found, then change the type of the European number to L */
                for(long characterIndexInSequence = characterAmountInSequence - 1; characterIndexInSequence >= 0; characterIndexInSequence--)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    if(currentType == CABidiTypeEN)
                    {
                        long backwardIndexInSequence;
                        for(backwardIndexInSequence = characterIndexInSequence - 1; backwardIndexInSequence >= 0; backwardIndexInSequence--)
                        {
                            size_t backwardIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, backwardIndexInSequence);
                            CABidiType backwardType = generator->typeArr[backwardIndexInString];
                            if(CABidiClassifyCategoryForType(backwardType) == CABidiCategoryStrong)
                            {
                                if(backwardType == CABidiTypeL)
                                    generator->typeArr[currentIndexInString] = CABidiTypeL;
                                break;
                            }
                        }
                        /* break exit */
                        if(backwardIndexInSequence < 0) /* check sos */
                            if(sos == CABidiTypeL)
                                generator->typeArr[currentIndexInString] = CABidiTypeL;
                    }
                }
                
                /* Resolving Neutral and Isolate Formatting Types */
                
                /* Rosolve Paired Brackets */
                tryToPairElement tryToPairStack[TryToPairBracketStackSize];
                size_t stackElementAmount = 0;
                
                size_t pairedBracketAmount = 0;
                for(long characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    UTF32Char currentCharacter = UTF32StringGetCharAtIndex(generator->string, currentIndexInString);
                    UTF32Char pairedChar;
                    CABidiPairedBracketType bracketType = CABidiGetPairedBracketType(currentCharacter, &pairedChar);
                    if(bracketType == CABidiPairedBracketTypeOpen)
                    {
                        if(stackElementAmount < TryToPairBracketStackSize)
                            tryToPairStack[stackElementAmount++] = (tryToPairElement){.pairedCloseCharacter = pairedChar, .index = characterIndexInSequence};
                        else
                            break; /* break 01 */
                    }
                    else if(bracketType == CABidiPairedBracketTypeClose)
                    {
                        for(long stackIndex = stackElementAmount - 1; stackIndex >= 0; stackIndex--)
                            if(tryToPairStack[stackIndex].pairedCloseCharacter == currentCharacter)
                            {
                                pairedBracketAmount++;
                                stackElementAmount = stackIndex;
                                break;  /* break 02 */
                            }
                        /* break 02 exit */
                    }
                }
                /* break 01 exit */
                
                pairedBracket *pairsInSequence = NULL; /* if pairedBracketAmount == 0, this is NULL */
                if(pairedBracketAmount > 0)
                {
                    size_t pairsInSequenceArrIndex = 0;
                    
                    if((pairsInSequence = malloc(sizeof(pairedBracket))) != NULL)
                    {
                        for(long characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                        {
                            size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                            UTF32Char currentCharacter = UTF32StringGetCharAtIndex(generator->string, currentIndexInString);
                            UTF32Char pairedChar;
                            CABidiPairedBracketType bracketType = CABidiGetPairedBracketType(currentCharacter, &pairedChar);
                            if(bracketType == CABidiPairedBracketTypeOpen)
                            {
                                if(stackElementAmount < TryToPairBracketStackSize)
                                    tryToPairStack[stackElementAmount++] = (tryToPairElement){.pairedCloseCharacter = pairedChar, .index = characterIndexInSequence};
                                else
                                    break; /* break 01 */
                            }
                            else if(bracketType == CABidiPairedBracketTypeClose)
                            {
                                for(long stackIndex = stackElementAmount - 1; stackIndex >= 0; stackIndex--)
                                    if(tryToPairStack[stackIndex].pairedCloseCharacter == currentCharacter)
                                    {
                                        pairsInSequence[pairsInSequenceArrIndex++] = (pairedBracket){.openIndex = tryToPairStack[stackIndex].index, .closeIndex = characterIndexInSequence};
                                        stackElementAmount = stackIndex;
                                        break;  /* break 02 */
                                    }
                                /* break 02 exit */
                            }
                        }
                    }
                    else
                    {
                        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorProcessPragraphLevel allocate pairsInSequence failed");
                        if(nonSpaceMarkIndexInSequenceArr != NULL) free(pairsInSequence);
                        CAIsolatingRunSequencesDataDestory(sequenceData);
                        CAMatchingExplictFormatDestory(matchingExplictFormat);
                        return;
                    }
                }
                
                /* N0 - Unicode Bidirectional Algorithm */
                /* Process bracket pairs in an isolating run */
                /* Within this scope, bidirectional types EN and AN are treated as R */
                for(size_t pairsIndex = 0; pairsIndex < pairedBracketAmount; pairsIndex++)
                {
                    size_t openPairIndexInSequence = pairsInSequence[pairsIndex].openIndex;
                    size_t closePairIndexInSequence = pairsInSequence[pairsIndex].closeIndex;
                    size_t openPairIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, openPairIndexInSequence);
                    size_t closePairIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, closePairIndexInSequence);
                    
                    /* If any strong type (either L or R) matching the embedding direction is found, set the type for both brackets in the pair to match the embedding direction */
                    bool isFoundMatchEmbbedingDirection = false;
                    bool hasStrongType = false;
                    for(size_t insidePairIndex = openPairIndexInSequence + 1; insidePairIndex < closePairIndexInSequence; insidePairIndex++)
                    {
                        size_t insideIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, insidePairIndex);
                        CABidiType insideType = generator->typeArr[insideIndexInString];
                        CABidiLevel insideLevel = generator->levelArr[insideIndexInString];
                        if(insideType == CABidiTypeL)
                        {
                            hasStrongType = true;
                            if(CABidiTypeL == EmbeddingDirection(insideLevel))
                            {
                                isFoundMatchEmbbedingDirection = true;
                                generator->typeArr[openPairIndexInString] = CABidiTypeL;
                                generator->typeArr[closePairIndexInString] = CABidiTypeL;
                                
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == openPairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = CABidiTypeL;
                                        break;
                                    }
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == closePairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = CABidiTypeL;
                                        break;
                                    }
                                
                                break;
                            }
                        }
                        else if(insideType == CABidiTypeR || insideType == CABidiTypeEN || insideType == CABidiTypeAN)
                        {
                            hasStrongType = true;
                            if(CABidiTypeR == EmbeddingDirection(insideLevel))
                            {
                                isFoundMatchEmbbedingDirection = true;
                                generator->typeArr[openPairIndexInString] = CABidiTypeR;
                                generator->typeArr[closePairIndexInString] = CABidiTypeR;
                                
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == openPairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = CABidiTypeR;
                                        break;
                                    }
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == closePairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = CABidiTypeR;
                                        break;
                                    }
                                
                                break;
                            }
                        }
                    }
                    /* break exit */
                    
                    if(!isFoundMatchEmbbedingDirection)
                    {
                        if(hasStrongType)
                        {
                            long formerIndexInSequence;
                            CABidiType firstStrongType = CABidiTypeL;   /* useless assignment */
                            for(formerIndexInSequence = openPairIndexInSequence - 1; formerIndexInSequence >= 0; formerIndexInSequence--)
                            {
                                firstStrongType = generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, formerIndexInSequence)];
                                if(CABidiClassifyCategoryForType(firstStrongType) == CABidiCategoryStrong || firstStrongType == CABidiTypeEN || firstStrongType == CABidiTypeAN)
                                {
                                    if(firstStrongType != CABidiTypeL)
                                        firstStrongType = CABidiTypeR;
                                    break;
                                }
                            }
                            if(formerIndexInSequence < 0)
                                firstStrongType = sos;
                            
                            CABidiLevel sequenceLevel = generator->levelArr[openPairIndexInString];
                            
                            if(EmbeddingDirection(sequenceLevel) != firstStrongType)
                            {
                                generator->typeArr[openPairIndexInString] = firstStrongType;
                                generator->typeArr[closePairIndexInString] = firstStrongType;
                                
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == openPairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = firstStrongType;
                                        break;
                                    }
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == closePairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = firstStrongType;
                                        break;
                                    }
                            }
                            else
                            {
                                generator->typeArr[openPairIndexInString] = EmbeddingDirection(sequenceLevel);
                                generator->typeArr[closePairIndexInString] = EmbeddingDirection(sequenceLevel);
                                
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == openPairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = EmbeddingDirection(sequenceLevel);
                                        break;
                                    }
                                for (size_t NSMPairArrIndex = 0; NSMPairArrIndex < nonSpaceMarkAmount; NSMPairArrIndex++)
                                    if(nonSpaceMarkIndexInSequenceArr[NSMPairArrIndex * 2 + 1] == closePairIndexInSequence)
                                    {
                                        size_t NSMIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NSMPairArrIndex * 2);
                                        generator->typeArr[NSMIndexInString] = EmbeddingDirection(sequenceLevel);
                                        break;
                                    }
                            }
                        }
                    }
                }
                
                /* N1 N2 - Unicode Bidirectional Algorithm */
                /* A sequence of NIs takes the direction of the surrounding strong text if the text on both sides has the same direction */
                /* European and Arabic numbers act as if they were R in terms of their influence on NIs */
                for(size_t characterIndexInSequence = 0; characterIndexInSequence < characterAmountInSequence; characterIndexInSequence++)
                {
                    size_t currentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, characterIndexInSequence);
                    CABidiType currentType = generator->typeArr[currentIndexInString];
                    
                    if(isExplicitDirectionalIsolates(currentType) || currentType == CABidiTypePDI || CABidiClassifyCategoryForType(currentType) == CABidiCategoryNeutral || currentType == CABidiTypeBN)
                    {
                        bool hasNI = (currentType != CABidiTypeBN);
                        size_t NIsBeginIndexInSequence = characterAmountInSequence;
                        size_t NIsEndIndexInSequence = NIsBeginIndexInSequence;
                        
                        LOOP
                        {
                            if(NIsEndIndexInSequence < characterAmountInSequence - 1)
                            {
                                size_t nextIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NIsEndIndexInSequence);
                                CABidiType nextType = generator->typeArr[nextIndexInString];
                                if(isExplicitDirectionalIsolates(nextType) || nextType == CABidiTypePDI || CABidiClassifyCategoryForType(nextType) == CABidiCategoryNeutral || nextType == CABidiTypeBN)
                                {
                                    if(nextType != CABidiTypeBN) hasNI = true;
                                    NIsEndIndexInSequence++;
                                    continue;
                                }
                            }
                            break;
                            /* continue point */
                        }
                        
                        if(hasNI)
                        {
                            CABidiType leftType;
                            if(NIsBeginIndexInSequence == 0)
                                leftType = sos;
                            else
                            {
                                leftType = generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NIsBeginIndexInSequence - 1)];
                                if(leftType == CABidiTypeEN || leftType == CABidiTypeAN)
                                    leftType = CABidiTypeR;
                            }
                            
                            CABidiType rightType;
                            if(NIsEndIndexInSequence == characterAmountInSequence - 1)
                                rightType = eos;
                            else
                            {
                                rightType = generator->typeArr[CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NIsEndIndexInSequence + 1)];
                                if(rightType == CABidiTypeEN || rightType == CABidiTypeAN)
                                    rightType = CABidiTypeR;
                            }
                            
                            /* security check */
                            if(CABidiClassifyCategoryForType(leftType) != CABidiCategoryStrong || CABidiClassifyCategoryForType(rightType) != CABidiCategoryStrong)
                            {
                                CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorProcessPragraphLevel process sequence of NIs failed by left/right type");
                                if(nonSpaceMarkIndexInSequenceArr != NULL) free(pairsInSequence);
                                if(pairsInSequence != NULL) free(pairsInSequence);
                                CAIsolatingRunSequencesDataDestory(sequenceData);
                                CAMatchingExplictFormatDestory(matchingExplictFormat);
                            }
                            
                            CABidiType takeDirection;
                            
                            if(leftType == rightType)
                                takeDirection = leftType;
                            else
                                takeDirection = EmbeddingDirection(generator->levelArr[currentIndexInString]);
                            
                            for(size_t NIsCurrentIndexInSequence = NIsBeginIndexInSequence; NIsCurrentIndexInSequence <= NIsEndIndexInSequence; NIsCurrentIndexInSequence++)
                            {
                                size_t NIsCurrentIndexInString = CAIsolatingRunSequencesDataGetCharacterIndex(sequenceData, sequenceIndex, NIsCurrentIndexInSequence);
                                generator->typeArr[NIsCurrentIndexInString] = takeDirection;
                            }
                        }
                        
                        characterIndexInSequence = NIsEndIndexInSequence;
                    }
                }
                
                if(nonSpaceMarkIndexInSequenceArr != NULL) free(pairsInSequence);
                if(pairsInSequence != NULL) free(pairsInSequence);
            }
            CAIsolatingRunSequencesDataDestory(sequenceData);
        }
        else
        {
            CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorProcessPragraphLevel allocate sequenceData failed");
            CAMatchingExplictFormatDestory(matchingExplictFormat);
            return;
        }
        
        CAMatchingExplictFormatDestory(matchingExplictFormat);
    }
    else
    {
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorProcessPragraphLevel allocate matchingExplictFormat failed");
        return;
    }
    
    /* Resolving Implicit Levels */
    
    /* I1 I2 - Unicode Bidirectional Algorithm */
    /* it is possible for text to end up at level max_depth+1 as a result of this process */
    for(size_t currentIndex = beginIndex; currentIndex <= endIndex; currentIndex++)
    {
        CABidiType currentType = generator->typeArr[currentIndex];
        CABidiLevel currentLevel = generator->levelArr[currentIndex];
        
        if(currentLevel % 2 == 0)
        {
            /* even */
            if(currentType == CABidiTypeR)
                generator->levelArr[currentIndex]++;
            else if(currentType == CABidiTypeEN || currentType == CABidiTypeAN)
                generator->levelArr[currentIndex] += 2;
        }
        else
        {
            /* odd */
            if(currentType == CABidiTypeR || currentType == CABidiTypeEN || currentType == CABidiTypeAN)
                generator->levelArr[currentIndex]++;
        }
    }
}

size_t *CABLGeneratorDisplayLine(CABLGeneratorRef generator, CFRange paragraphRange, CFRange lineRange)
{
    if(generator != NULL &&
       paragraphRange.length > 0 &&
       lineRange.length > 0 &&
       CFMaxRange(paragraphRange) <= generator->length &&
       lineRange.location >= paragraphRange.location &&
       CFMaxRange(lineRange) <= CFMaxRange(paragraphRange))
    {
        CABidiLevel paragraphLevel = CABLGeneratorResolvePragraphEmbeddingLevel(generator, paragraphRange.location, CFMaxRange(paragraphRange) - 1);
        
        size_t lineLength = lineRange.length;
        size_t lineIndexPlusToStringIndex = lineRange.location;
        
        CABidiLevel *lineLevel;
        if((lineLevel = malloc(sizeof(CABidiLevel) * lineLength)) != NULL)
        {
            memcpy(lineLevel, generator->typeArr + lineRange.location, sizeof(CABidiLevel) * lineLength);
            
            /* L1 - Unicode Bidirectional Algorithm */
            for (size_t currentLineIndex = 0; currentLineIndex < lineLength; currentLineIndex++)
            {
                UTF32Char currentCharacter = UTF32StringGetCharAtIndex(generator->string, currentLineIndex + lineIndexPlusToStringIndex);
                CABidiType currentType = CABidiClassifyTypeForCharacter(currentCharacter);
            
                if(currentType == CABidiTypeB || currentType == CABidiTypeS)
                    lineLevel[currentLineIndex] = paragraphLevel;
            }
            for (size_t currentLineIndex = 0; currentLineIndex < lineLength; currentLineIndex++)
            {
                UTF32Char currentCharacter = UTF32StringGetCharAtIndex(generator->string, currentLineIndex + lineIndexPlusToStringIndex);
                CABidiType currentType = CABidiClassifyTypeForCharacter(currentCharacter);
                
                if(isExplicitDirectionalIsolates(currentType) || currentType == CABidiTypePDI || isExplicitDirectionalEmbeddings(currentType) || currentType == CABidiTypePDF || currentType == CABidiTypeBN || currentType == CABidiTypeWS)
                {
                    size_t sequencesBeginIndex = currentLineIndex;
                    size_t sequencesEndIndex = sequencesBeginIndex;
                    CABidiType nextType = CABidiTypeL;  /* useless assginment here */
                    
                    LOOP
                    {
                        if(sequencesEndIndex == lineLength - 1)
                            break;
                        UTF32Char nextCharacter = UTF32StringGetCharAtIndex(generator->string, sequencesEndIndex + 1);
                        nextType = CABidiClassifyTypeForCharacter(nextCharacter);
                        if(isExplicitDirectionalIsolates(nextType) || nextType == CABidiTypePDI || isExplicitDirectionalEmbeddings(nextType) || nextType == CABidiTypePDF || nextType == CABidiTypeBN || nextType == CABidiTypeWS)
                            sequencesEndIndex++;
                        else
                            break;
                    }
                    
                    if(sequencesEndIndex == lineLength - 1 || nextType == CABidiTypeB || nextType == CABidiTypeS)
                        for (size_t currentSequenceIndex = sequencesBeginIndex; currentSequenceIndex <= sequencesEndIndex; currentSequenceIndex++)
                            lineLevel[currentSequenceIndex] = paragraphLevel;
                    
                    currentLineIndex = sequencesEndIndex;
                }
            }
            
            /* L2 - Unicode Bidirectional Algorithm */
            size_t *result;
            if((result = malloc(sizeof(size_t) * lineLength)) != NULL)
            {
                CABidiLevel highestLevel = 0;
                for(size_t lineIndex = 0; lineIndex < lineLength; lineIndex++)
                    if(lineLevel[lineIndex] > highestLevel) highestLevel = lineLevel[lineIndex];
                
                CABidiLevel leastLevel = max_depth + 1;
                for(size_t lineIndex = 0; lineIndex < lineLength; lineIndex++)
                    if(lineLevel[lineIndex] < highestLevel) leastLevel = lineLevel[lineIndex];
                
                CABidiLevel leastOddLevel;
                if(leastLevel % 2 == 0)
                {
                    if(leastLevel == 0)
                        leastOddLevel = 1;
                    else
                        leastOddLevel = leastLevel - 1;
                }
                else
                    leastOddLevel = leastLevel;
                
                for(size_t lineIndex = 0; lineIndex < lineLength; lineIndex++)
                    result[lineIndex] = lineIndex + lineIndexPlusToStringIndex;
                
                for(size_t noLessThanLevel = highestLevel; noLessThanLevel >= leastOddLevel; noLessThanLevel--)
                    for(size_t lineIndex = 0; lineIndex < lineLength; lineIndex++)
                        if(lineLevel[lineIndex] >= noLessThanLevel)
                        {
                            size_t sequenceBeginIndexInLine = lineIndex;
                            size_t sequenceEndIndexInLine = sequenceBeginIndexInLine;
                            
                            LOOP
                            {
                                if(sequenceEndIndexInLine == lineLength - 1) break;
                                if(lineLevel[sequenceEndIndexInLine + 1] >= noLessThanLevel)
                                    sequenceEndIndexInLine++;
                                else
                                    break;
                            }
                            
                            size_t sequenceLength = sequenceEndIndexInLine - sequenceBeginIndexInLine + 1;
                            for(size_t reverseIndex = sequenceBeginIndexInLine; reverseIndex <= sequenceLength / 2; reverseIndex++)
                            {
                                size_t temp = result[reverseIndex];
                                result[reverseIndex] = result[sequenceEndIndexInLine - (reverseIndex - sequenceBeginIndexInLine)];
                                result[sequenceEndIndexInLine - (reverseIndex - sequenceBeginIndexInLine)] = temp;
                            }
                            
                            lineIndex = sequenceEndIndexInLine;
                        }
                
                free(lineLevel);
                
                /* L3 - Unicode Bidirectional Algorithm */
#warning lack of Combining mark process
                
                /* L4 - Unicode Bidirectional Algorithm */
#warning lack of mirror process
                
                return result;
            }
            free(lineLevel);
        }
    }
    return NULL;
}

static CABidiLevel CABLGeneratorResolvePragraphEmbeddingLevel(CABLGeneratorRef generator, size_t beginIndex, size_t endIndex)
{
    CAMatchingExplictFormatRef matchingExplictFormat;
    if((matchingExplictFormat = CABLGeneratorResolveMatchingExplictFormat(generator, beginIndex, endIndex)) != NULL)
    {
        /* resolve paragraph level */
        
        CABidiLevel paragraphLevel = 0;
        for(size_t index = beginIndex; index <= endIndex; index++)
        {
            CABidiType currentType = CABidiClassifyTypeForCharacter(UTF32StringGetCharAtIndex(generator->string, index));
            if(currentType == CABidiTypeAL || currentType == CABidiTypeR)
            { paragraphLevel = 1; break; }
            else if(isExplicitDirectionalIsolates(currentType))
            {
                size_t matchingPDIIndex;
                if(CAMatchingExplictFormatLocateMatchingPDI(matchingExplictFormat, index, &matchingPDIIndex))
                    index = matchingPDIIndex;   // not plus 1, consider about it
                else
                    index = endIndex;
            }
        }
        CAMatchingExplictFormatDestory(matchingExplictFormat);
        return paragraphLevel;
    }
    else
    {
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABLGeneratorResolvePragraphEmbeddingLevel create matchingExplictFormat failed");
    }
    return 0;
}

size_t CABLGeneratorGetParagraphAmount(CABLGeneratorRef generator)
{
    size_t result = 0;
    
    size_t beginIndex = 0, endIndex;
    size_t length = generator->length;
    do
    {
        endIndex = beginIndex;
        
        while(CABidiClassifyTypeForCharacter(UTF32StringGetCharAtIndex(generator->string, endIndex)) != CABidiTypeB && endIndex < length - 1) endIndex++;
        
        /* handle CRLF */
        if(endIndex < length - 1
           && UTF32StringGetCharAtIndex(generator->string, endIndex) == '\r'
           && UTF32StringGetCharAtIndex(generator->string, endIndex + 1) == '\n')
            endIndex++;
        
        result++;
        
        beginIndex = endIndex + 1;
    }while(beginIndex < length);
    
    return result;
}

CFRange CABLGeneratorGetParagraphRange(CABLGeneratorRef generator, size_t paragraphIndex)
{
    size_t currentIndex = 0;
    
    size_t beginIndex = 0, endIndex;
    
    size_t length = generator->length;
    do
    {
        endIndex = beginIndex;
        
        while(CABidiClassifyTypeForCharacter(UTF32StringGetCharAtIndex(generator->string, endIndex)) != CABidiTypeB && endIndex < length - 1) endIndex++;
        
        /* handle CRLF */
        if(endIndex < length - 1
           && UTF32StringGetCharAtIndex(generator->string, endIndex) == '\r'
           && UTF32StringGetCharAtIndex(generator->string, endIndex + 1) == '\n')
            endIndex++;
        
        if(currentIndex++ == paragraphIndex) return CFRangeMake(beginIndex, endIndex - beginIndex + 1);
        
        beginIndex = endIndex + 1;
    }while(beginIndex < length);
    return CFRangeNotFound;
}

void CABLGeneratorDestory(CABLGeneratorRef generator)
{
    if(generator == NULL) return;
    UTF32StringDestory(generator->string);
    free(generator->levelArr);
    free(generator->typeArr);
    free(generator);
}

#pragma mark - CAIsolatingRunSequencesData (ADT)

typedef struct CALevelRun
{
    size_t beginIndex, endIndex;
}CALevelRun;

typedef struct CAIsolatingRunSequence
{
    size_t amount;
    CALevelRun *levelRunArr;
}CAIsolatingRunSequence;

struct CAIsolatingRunSequencesData
{
    CABLGeneratorRef generator;     // CAIsolatingRunSequencesData will not try to retain it
    size_t paragraphBeginIndex, paragrapEndIndex;
    size_t amount;
    CAIsolatingRunSequence *isolatingRunSequenceArr;
};

static CAIsolatingRunSequencesDataRef CAIsolatingRunSequencesDataCreate(CABLGeneratorRef generator, CAMatchingExplictFormatRef matching, size_t beginIndex, size_t endIndex)
{
    CAIsolatingRunSequencesDataRef result;
    if((result = malloc(sizeof(struct CAIsolatingRunSequencesData))) != NULL)
    {
        result->amount = 0;
        
        size_t levelRunBeginIndex = beginIndex;
        size_t levelRunEndIndex;
        do
        {
            levelRunEndIndex = levelRunBeginIndex;
            while (generator->levelArr[levelRunEndIndex + 1] == generator->levelArr[levelRunEndIndex])
                levelRunEndIndex++;
            
            if(!CAMatchingExplictFormatLocateMatchingIsolateInitializer(matching, levelRunEndIndex, NULL))
                result->amount++;
            
            levelRunBeginIndex = levelRunEndIndex + 1;
        }while(levelRunBeginIndex <= endIndex);
        
        if((result->isolatingRunSequenceArr = malloc(sizeof(CAIsolatingRunSequence) * result->amount)) != NULL)
        {
            bool failedCheck = false;
            
            size_t sequenceArrIndex = 0;
            
            size_t levelRunBeginIndex = beginIndex;
            size_t levelRunEndIndex;
            do
            {
                levelRunEndIndex = levelRunBeginIndex;
                while (levelRunEndIndex<endIndex && generator->levelArr[levelRunEndIndex + 1] == generator->levelArr[levelRunEndIndex])
                    levelRunEndIndex++;
                
                if(!CAMatchingExplictFormatLocateMatchingIsolateInitializer(matching, levelRunEndIndex, NULL))
                {
                    result->isolatingRunSequenceArr[sequenceArrIndex].amount = 1;
                    
                    size_t lastCharacter = levelRunEndIndex; /* isolate sequence begin level run end character index */
                    size_t lastCharacterMatch;
                    
                    while(CAMatchingExplictFormatLocateMatchingPDI(matching, lastCharacter, &lastCharacterMatch))
                    {
                        result->isolatingRunSequenceArr[sequenceArrIndex].amount++;
                        
                        lastCharacter = lastCharacterMatch;
                        while (lastCharacter < endIndex && generator->levelArr[lastCharacter + 1] == generator->levelArr[lastCharacter])
                            lastCharacter++;
                    }
                    
                    if((result->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr = malloc(sizeof(CALevelRun) * result->isolatingRunSequenceArr[sequenceArrIndex].amount)) != NULL)
                    {
                        size_t firstCharacter = levelRunBeginIndex;
                        size_t lastCharacter = levelRunEndIndex;
                        
                        size_t levelRunArrIndex = 0;
                        LOOP
                        {
                            result->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[levelRunArrIndex++] = (CALevelRun){.beginIndex = firstCharacter, .endIndex = lastCharacter};
                            
                            size_t lastCharacterMatch;
                            if(CAMatchingExplictFormatLocateMatchingPDI(matching, lastCharacter, &lastCharacterMatch))
                            {
                                firstCharacter = lastCharacterMatch;
                                
                                lastCharacter = firstCharacter;
                                while (lastCharacter < endIndex && generator->levelArr[lastCharacter + 1] == generator->levelArr[lastCharacter])
                                    lastCharacter++;
                            }
                            else
                                break;
                        }
                    }
                    else
                    {
                        failedCheck = true;
                        /* dealloc inside levelRuns data in each isolate sequence here, then break out */
                        
                        for(size_t index = 0; index < sequenceArrIndex; index++)
                            free(result->isolatingRunSequenceArr[index].levelRunArr);
                        break;
                    }
                    sequenceArrIndex++;
                }
                
                levelRunBeginIndex = levelRunEndIndex + 1;
            }while(levelRunBeginIndex <= endIndex);
            
            if(!failedCheck)
            {
                result->generator = generator;
                result->paragraphBeginIndex = beginIndex;
                result->paragrapEndIndex = endIndex;
                return result;
            }
            
            /* at here inner isolate sequnece is able to free */
            
            free(result->isolatingRunSequenceArr);
        }
        free(result);
    }
    return NULL;
}

static void CAIsolatingRunSequencesDataDestory(CAIsolatingRunSequencesDataRef data)
{
    if(data == NULL) return;
    for(size_t index = 0; index < data->amount; index++)
        free(data->isolatingRunSequenceArr[index].levelRunArr);
    free(data->isolatingRunSequenceArr);
    free(data);
}

static size_t CAIsolatingRunSequencesDataGetCharacterIndex(CAIsolatingRunSequencesDataRef data, size_t sequenceArrIndex, size_t characterIndexInThisSequence)
{
    if(data == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAIsolatingRunSequencesData NULL GetCharacterIndex");
    else if(sequenceArrIndex >= data->amount)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAIsolatingRunSequencesDataGetCharacterIndex at sequenceIndex %zu with sequences amount %zu", sequenceArrIndex, data->amount);
    else
    {
        size_t currentCharacterIndexInThisSequence = 0;
        for(size_t index = 0; index < data->isolatingRunSequenceArr[sequenceArrIndex].amount; index++)
            if(data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].endIndex - data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].beginIndex + currentCharacterIndexInThisSequence >= characterIndexInThisSequence)
                return characterIndexInThisSequence - currentCharacterIndexInThisSequence + data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].beginIndex;
            else
                currentCharacterIndexInThisSequence += data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].endIndex - data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].beginIndex + 1;
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAIsolatingRunSequencesDataGetCharacterIndex at sequenceIndex %zu characterIndex %zu with character amount in sequence %zu", sequenceArrIndex, characterIndexInThisSequence, currentCharacterIndexInThisSequence);
    }
    return SIZE_MAX;
}

static size_t CAIsolatingRunSequencesDataGetSequenceAmount(CAIsolatingRunSequencesDataRef data)
{
    if(data == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAIsolatingRunSequencesData NULL GetSequenceAmount");
        return SIZE_MAX;
    }
    return data->amount;
}

static size_t CAIsolatingRunSequencesDataGetCharacterAmountInSequence(CAIsolatingRunSequencesDataRef data, size_t sequenceArrIndex)
{
    if(data == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAIsolatingRunSequencesData NULL GetCharacterAmountInSequence");
    else if(sequenceArrIndex >= data->amount)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAIsolatingRunSequencesDataGetCharacterAmountInSequence at sequenceIndex %zu with sequences amount %zu", sequenceArrIndex, data->amount);
    else
    {
        size_t result = 0;
        for(size_t index = 0; index < data->isolatingRunSequenceArr[sequenceArrIndex].amount; index++)
            result += data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].endIndex - data->isolatingRunSequenceArr[sequenceArrIndex].levelRunArr[index].beginIndex + 1;
        return result;
    }
    return SIZE_MAX;
}

#pragma mark - Matching Explict Format (ADT)

#define CAMatchingExplictFormatInitializedArraySize 64
#define CAMatchingExplictFormatIncreasedArraySize 24

/**
 @return If not found, return SIZE_MAX <limits.h>
 */
static size_t CABLGeneratorFindMatchingPDI(CABLGeneratorRef generator, size_t isolateInitiatorIndex, size_t paragraphEndIndex);

/**
 @return If not found, return SIZE_MAX <limits.h>
 */
static size_t CABLGeneratorFindMatchingPDF(CABLGeneratorRef generator, size_t embeddingInitiatorIndex, size_t paragraphEndIndex, size_t *matchingPDIArr, size_t matchingPDIArrLength);


struct CAMatchingExplictFormat
{
    size_t matchingPDIArrAllocatedSize;
    size_t matchingPDIArrLength;
    size_t *matchingPDIArr;                 // first (LRI, RLI, or FSI) second (PDI)
    size_t matchingPDFArrAllocatedSize;
    size_t matchingPDFArrLength;
    size_t *matchingPDFArr;                 // first (LRE, RLE, LRO, or RLO) second (PDF)
};

static bool CAMatchingExplictFormatLocateMatchingPDI(CAMatchingExplictFormatRef matchingExplictFormat, size_t isolateInitializer, size_t *matchingPDIIndex)
{
    if(matchingExplictFormat == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAMatchingExplictFormat NULL LocateMatchingPDI");
        return false;
    }
    for(size_t index = 0; index < matchingExplictFormat->matchingPDIArrLength / 2; index++)
        if(matchingExplictFormat->matchingPDIArr[index * 2] == isolateInitializer)
        {
            if(matchingPDIIndex != NULL) *matchingPDIIndex = matchingExplictFormat->matchingPDIArr[index * 2 + 1];
            return true;
        }
    return false;
}

static bool CAMatchingExplictFormatLocateMatchingIsolateInitializer(CAMatchingExplictFormatRef matchingExplictFormat, size_t PDI_Index, size_t *isolateIntializerIndex)
{
    if(matchingExplictFormat == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAMatchingExplictFormat NULL LocateMatchingPDI");
        return false;
    }
    for(size_t index = 0; index < matchingExplictFormat->matchingPDIArrLength / 2; index++)
        if(matchingExplictFormat->matchingPDIArr[index * 2 + 1] == PDI_Index)
        {
            if(isolateIntializerIndex != NULL) *isolateIntializerIndex = matchingExplictFormat->matchingPDIArr[index * 2];
            return true;
        }
    return false;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
static bool CAMatchingExplictFormatLocateMatchingPDF(CAMatchingExplictFormatRef matchingExplictFormat, size_t embeddingInitializer, size_t *matchingPDFIndex)
{
    if(matchingExplictFormat == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CAMatchingExplictFormat NULL LocateMatchingPDF");
        return false;
    }
    for(size_t index = 0; index < matchingExplictFormat->matchingPDFArrLength / 2; index++)
        if(matchingExplictFormat->matchingPDFArr[index * 2] == embeddingInitializer)
        {
            if(matchingPDFIndex != NULL) *matchingPDFIndex = matchingExplictFormat->matchingPDFArr[index * 2 + 1];
            return true;
        }
    return false;
}
#pragma clang diagnostic pop

static CAMatchingExplictFormatRef CABLGeneratorResolveMatchingExplictFormat(CABLGeneratorRef generator, size_t beginIndex, size_t endIndex)
{
    CAMatchingExplictFormatRef result;
    if((result = malloc(sizeof(struct CAMatchingExplictFormat))) != NULL)
    {
        if((result->matchingPDIArr = malloc(sizeof(size_t) * CAMatchingExplictFormatInitializedArraySize)) != NULL)
        {
            if((result->matchingPDFArr = malloc(sizeof(size_t) * CAMatchingExplictFormatInitializedArraySize)) != NULL)
            {
                result->matchingPDFArrAllocatedSize = CAMatchingExplictFormatInitializedArraySize;
                result->matchingPDIArrAllocatedSize = CAMatchingExplictFormatInitializedArraySize;
                result->matchingPDIArrLength = 0u;
                result->matchingPDFArrLength = 0u;
                
                bool failedCheck = false;
                
                for(size_t index = beginIndex; index <= endIndex && !failedCheck; index++)
                    if(isExplicitDirectionalIsolates(generator->typeArr[index]))
                    {
                        size_t matchingPDI = CABLGeneratorFindMatchingPDI(generator, index, endIndex);
                        if(matchingPDI != SIZE_MAX)
                        {
                            if(result->matchingPDIArrAllocatedSize - result->matchingPDIArrLength < 2)
                            {
                                size_t *temp = realloc(result->matchingPDIArr, sizeof(size_t) * result->matchingPDIArrAllocatedSize + CAMatchingExplictFormatIncreasedArraySize);
                                if(temp != NULL)
                                {
                                    result->matchingPDIArr = temp;
                                    result->matchingPDIArrAllocatedSize = result->matchingPDIArrAllocatedSize + CAMatchingExplictFormatIncreasedArraySize;
                                }
                                else
                                    failedCheck = true;
                            }
                            if(!failedCheck)
                            {
                                result->matchingPDIArr[result->matchingPDIArrLength++] = index;
                                result->matchingPDIArr[result->matchingPDIArrLength++] = matchingPDI;
                            }
                        }
                    }
                
                for(size_t index = beginIndex; index <= endIndex && !failedCheck; index++)
                    if(isExplicitDirectionalEmbeddings(generator->typeArr[index]))
                    {
                        size_t matchingPDF = CABLGeneratorFindMatchingPDF(generator, index, endIndex, result->matchingPDIArr, result->matchingPDIArrLength);
                        if(matchingPDF != SIZE_MAX)
                        {
                            if(result->matchingPDFArrAllocatedSize - result->matchingPDFArrLength < 2)
                            {
                                size_t *temp = realloc(result->matchingPDFArr, sizeof(size_t) * result->matchingPDFArrAllocatedSize + CAMatchingExplictFormatIncreasedArraySize);
                                if(temp != NULL)
                                {
                                    result->matchingPDFArr = temp;
                                    result->matchingPDFArrAllocatedSize = result->matchingPDFArrAllocatedSize + CAMatchingExplictFormatIncreasedArraySize;
                                }
                                else
                                    failedCheck = true;
                            }
                            if(!failedCheck)
                            {
                                result->matchingPDFArr[result->matchingPDFArrLength++] = index;
                                result->matchingPDFArr[result->matchingPDFArrLength++] = matchingPDF;
                            }
                        }
                    }
                
                if(!failedCheck)
                    return result;
                
                free(result->matchingPDFArr);
            }
            free(result->matchingPDIArr);
        }
        free(result);
    }
    return NULL;
}

static void CAMatchingExplictFormatDestory(CAMatchingExplictFormatRef matchingExplictFormat)
{
    if(matchingExplictFormat == NULL) return;
    free(matchingExplictFormat->matchingPDIArr);
    free(matchingExplictFormat->matchingPDFArr);
    free(matchingExplictFormat);
}

static size_t CABLGeneratorFindMatchingPDI(CABLGeneratorRef generator, size_t isolateInitiatorIndex, size_t paragraphEndIndex)
{
    size_t count = 1;
    size_t currentIndex = isolateInitiatorIndex + 1;
    
    while (currentIndex <= paragraphEndIndex)
    {
        if(isExplicitDirectionalIsolates(generator->typeArr[currentIndex]))
            count++;
        else if(generator->typeArr[currentIndex] == CABidiTypePDI)
            if(--count == 0) return currentIndex;
        currentIndex++;
    }
    return SIZE_MAX;
}

static size_t CABLGeneratorFindMatchingPDF(CABLGeneratorRef generator, size_t embeddingInitiatorIndex, size_t paragraphEndIndex, size_t *matchingPDIArr, size_t matchingPDIArrLength)
{
    size_t count = 1;
    size_t currentIndex = embeddingInitiatorIndex + 1;
    
    while (currentIndex <= paragraphEndIndex)
    {
        if(isExplicitDirectionalIsolates(generator->typeArr[currentIndex]))
        {
           bool foundMatchingPDI = false;
           for(size_t index = 0; index < matchingPDIArrLength / 2; index++)
               if(matchingPDIArr[index * 2] == currentIndex)
               {
                   foundMatchingPDI = true;
                   currentIndex = matchingPDIArr[index * 2 + 1];
               }
           if(!foundMatchingPDI)
               currentIndex = paragraphEndIndex;
        }
        else if(generator->typeArr[currentIndex] == CABidiTypePDI)
        {
            for(size_t index = 0; index < matchingPDIArrLength / 2; index++)
                if(matchingPDIArr[index * 2 + 1] == currentIndex)
                    return SIZE_MAX;
        }
        else if(isExplicitDirectionalEmbeddings(generator->typeArr[currentIndex]))
            count++;
        else if(generator->typeArr[currentIndex] == CABidiTypePDF)
            if(--count == 0) return currentIndex;
        currentIndex++;
    }
    return SIZE_MAX;
}
