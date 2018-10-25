

//
//  CAPunycode.c
//  TextKit
//
//  Created by Bill Sun on 2018/10/23.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#pragma mark - Include part
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include "CFDebug.h"
#include "CFType.h"
#include "CAPunycode.h"
#include "UTF32String.h"

#pragma mark - Macro configuration
#define CAPunycodeDecodeInitialTempArraySize 512
#define CAPunycodeDecodeInitialTempArrayIncreasedAmount 64
#define LOOP for(;;)

#pragma mark - Punycode static constant configuration
static const unsigned int damp = 700;
static const unsigned int base = 36;
static const unsigned int tmin = 1;
static const unsigned int tmax = 26;
static const unsigned int skew = 38;
static const unsigned int initial_bias = 72;
static const unsigned int initial_value = 128;      // inital_n 0x80
static const unsigned int overflowValue = UINT_MAX;

#pragma mark - static function declearation
static unsigned int CAPunycodeBiasAdaptation(unsigned int delta, unsigned int numpoints, bool firstTime);
static const UTF8Char * CAPunycodeStrrchr(const UTF8Char *source, UTF8Char ch);
static size_t CAPunycodeStrlen(const UTF8Char *source);
static bool CAPunycodeIsBasicCodePoint(UTF32Char codePoint);
static unsigned int CAPunycodeDecodeDigit(UTF8Char ch);
static size_t CAPunycodeUnicodeStrlen(const UTF32Char *source);
static UTF32Char CAPunycodeEecodeDigit(unsigned int value);

/**
 Punycode bias cuculation function

 @param delta the length of the run of non-insertion states preceeding the insertion states (the increased part of accumulated)
 @param numpoints total number of code points encoded/decoded so far (including the one corresponding to this delta itself, and including the basic code points).
        actually the value of possible insertion points
 @param firstTime first time of decode insert the non-based code point
 @return caculated bias value
 */
static unsigned int CAPunycodeBiasAdaptation(unsigned int delta, unsigned int numpoints, bool firstTime)
{
    if(firstTime) delta = delta / damp;
    else delta = delta / 2;                 // delta >> 1 is also acceptable
    
    delta = delta + (delta / numpoints);
    
    unsigned int divisionTimes;
    for (divisionTimes = 0;  delta > ((base - tmin) * tmax) / 2;  divisionTimes++)
        delta /= base - tmin;
    
    return divisionTimes * base + (((base - tmin + 1) * delta) / (delta + skew));
}

/**
 This follow the RFC3492 tutorial to decode the Punycode String

 @param source a ASCII string terminated will '\0'
 @return a UTF32String with ownership
 */
UTF32StringRef CAPunycodeDecodeASCIIString(const UTF8Char *source)
{
    size_t storeArraySize = CAPunycodeDecodeInitialTempArraySize;   // 512
    const size_t storeArrayIncreaseAmount = CAPunycodeDecodeInitialTempArrayIncreasedAmount;    // 64
    size_t currentStoreStringLength = 0;
    UTF32Char *longStringArr;
    if((longStringArr = malloc(sizeof(UTF32Char) * storeArraySize)) != NULL)
    {
        const UTF8Char *lastDelimiter = CAPunycodeStrrchr(source, '-');
        // consume all code points before the last delimiter (if there is one)
        // and copy them to output, fail on any non-basic code point
        if(lastDelimiter == NULL) lastDelimiter = source + CAPunycodeStrlen(source);
        for(const UTF8Char *current = source; current < lastDelimiter; current++)
        {
            if(!CAPunycodeIsBasicCodePoint(*current))
            {
                free(longStringArr);
                CFDebugLog("CAPunycodeDecodeASCIIString not valid punycode string");
                return NULL;
            }
            if(currentStoreStringLength > storeArraySize)
            {
                UTF32Char *temp = realloc(longStringArr, sizeof(UTF32Char) * (storeArraySize + storeArrayIncreaseAmount));
                if(temp == NULL)
                {
                    free(longStringArr);
                    CFDebugLog("CAPunycodeDecodeASCIIString allcate temp array failed");
                    return NULL;
                }
                longStringArr = temp;
                storeArraySize += storeArrayIncreaseAmount;
            }
            longStringArr[currentStoreStringLength++] = *current;
        }
        
        lastDelimiter++; // points to the point if non-basic codepoint coded position
        
        size_t index = lastDelimiter - source;
        size_t sourceStringLength = CAPunycodeStrlen(source);
        
        unsigned int bias = initial_bias;   // 72
        unsigned int insertIndex = 0;
        unsigned int value = initial_value; // n 0x80, curresponding to current least-non-basic-codepoint value
        
        while (index < sourceStringLength)
        {
            size_t oldInsertIndex = insertIndex;    // oldInsertIndex is only used to whether it is the first time of non-basic-codepoint transform
            unsigned int weight = 1;
            unsigned int timesOfBase = base;    // 36
            
            LOOP
            {
                // begin consume a code point, or fail if there was none to consume
                
                if(index >= sourceStringLength)
                {
                    // punycode_bad_code
                    free(longStringArr);
                    CFDebugLog("CAPunycodeDecodeASCIIString badcode");
                    return NULL;
                }
                
                unsigned int digit = CAPunycodeDecodeDigit(source[index++]);
                
                if(digit >= base)
                {
                    // punycode badcode
                    free(longStringArr);
                    return NULL;
                }
                
                if (digit > (overflowValue - insertIndex) / weight)
                {
                    // punycode_overflow
                    free(longStringArr);
                    CFDebugLog("CAPunycodeDecodeASCIIString failed on overflow");
                    return NULL;
                }
                
                insertIndex = insertIndex + digit * weight;
                
                unsigned int threadhold;
                if(timesOfBase <= bias + tmin) threadhold = tmin;
                else if(timesOfBase >= bias + tmax) threadhold = tmax;
                else threadhold = timesOfBase - bias;     // threadhold = base * (j + 1) - bias (j is current loop index)
                
                if(digit < threadhold) break;
                
                if (weight > overflowValue / (base - threadhold))
                {
                    // punycode_overflow
                    free(longStringArr);
                    CFDebugLog("CAPunycodeDecodeASCIIString failed on overflow");
                    return NULL;
                }
                
                weight *= (base - threadhold);
                
                timesOfBase += base;
            }
            
            bias = CAPunycodeBiasAdaptation((unsigned int)(insertIndex - oldInsertIndex), (unsigned int)(currentStoreStringLength + 1), oldInsertIndex == 0);
            
//            bool firstTimeOfNonBasicCodePoint = oldAccumulated == 0;
//            unsigned int delta = (unsigned int)(accumulated - oldAccumulated);  // delta of Accumulated
//            unsigned int possibleInsertPointsAmount = (unsigned int)(currentStoreStringLength + 1);
//
//            if(firstTimeOfNonBasicCodePoint) delta = delta / damp;
//            else delta = delta / 2;
//
//            delta = delta + (delta / possibleInsertPointsAmount);
//
//            unsigned int divisionTimes;
//            for (divisionTimes = 0;  delta > ((base - tmin) * tmax) / 2;  divisionTimes++)
//                delta /= base - tmin;
//
//            bias = divisionTimes * base + (((base - tmin + 1) * delta) / (delta + skew));
            
            if (insertIndex / (currentStoreStringLength + 1) > overflowValue - value)
            {
                // punycode_overflow
                free(longStringArr);
                CFDebugLog("CAPunycodeDecodeASCIIString failed on overflow");
                return NULL;
            }
            
            value = (unsigned int)(value + insertIndex / (currentStoreStringLength + 1));
            insertIndex = insertIndex % (currentStoreStringLength + 1);
            
            if(CAPunycodeIsBasicCodePoint(value))
            {
                free(longStringArr);
                CFDebugLog("CAPunycodeDecodeASCIIString failed on punycode basic code point");
                return NULL;
            }
            
            if(currentStoreStringLength > storeArraySize)
            {
                UTF32Char *temp = realloc(longStringArr, sizeof(UTF32Char) * (storeArraySize + storeArrayIncreaseAmount));
                if(temp == NULL)
                {
                    free(longStringArr);
                    CFDebugLog("CAPunycodeDecodeASCIIString allcate temp array failed");
                    return NULL;
                }
                longStringArr = temp;
                storeArraySize += storeArrayIncreaseAmount;
            }
            
            if(currentStoreStringLength > 0)
            {
                for(long stringArrIndex = currentStoreStringLength - 1; stringArrIndex >= insertIndex; stringArrIndex--)
                    longStringArr[stringArrIndex + 1] = longStringArr[stringArrIndex];
            }
            
            longStringArr[insertIndex] = value;
            
            currentStoreStringLength++;
            
            insertIndex++;
        }
        
        UTF32StringRef result;
        if((result = UTF32StringCreateWithCharacterArray(longStringArr, currentStoreStringLength)) != NULL)
        {
            free(longStringArr);
            return result;
        }
        
        CFDebugLog("CAPunycodeDecodeASCIIString create UTF32String failed");
        
        free(longStringArr);
    }
    return NULL;
}

UTF32StringRef CAPunycodeEecodeUnicodeString(const UTF32Char *source)
{
    size_t storeArraySize = CAPunycodeDecodeInitialTempArraySize;   // 512
    const size_t storeArrayIncreaseAmount = CAPunycodeDecodeInitialTempArrayIncreasedAmount;    // 64
    size_t currentStoreStringLength = 0;
    UTF32Char *longStringArr;
    if((longStringArr = malloc(sizeof(UTF32Char) * storeArraySize)) != NULL)
    {
        
        size_t sourceStringLength = CAPunycodeUnicodeStrlen(source);
        
        unsigned int value = initial_value;
        unsigned int currentOutputpoints = 0;     // delta of accumulated
        unsigned int bias = initial_bias;
        
        
        unsigned int numberOfBasicCodePoint = 0;
        for(size_t loopIndex = 0; loopIndex < sourceStringLength; loopIndex++)
            if((CAPunycodeIsBasicCodePoint(source[loopIndex])))
                numberOfBasicCodePoint++;
        
        if(numberOfBasicCodePoint > 0)
        {
            for(size_t loopIndex = 0; loopIndex < sourceStringLength; loopIndex++)
                if((CAPunycodeIsBasicCodePoint(source[loopIndex])))
                {
                    if(currentStoreStringLength > storeArraySize)
                    {
                        UTF32Char *temp = realloc(longStringArr, sizeof(UTF32Char) * (storeArraySize + storeArrayIncreaseAmount));
                        if(temp == NULL)
                        {
                            free(longStringArr);
                            CFDebugLog("CAPunycodeEecodeUnicodeString allcate temp array failed");
                            return NULL;
                        }
                        longStringArr = temp;
                        storeArraySize += storeArrayIncreaseAmount;
                    }
                    
                    longStringArr[currentStoreStringLength++] = source[loopIndex];
                }
            
            if(currentStoreStringLength > storeArraySize)
            {
                UTF32Char *temp = realloc(longStringArr, sizeof(UTF32Char) * (storeArraySize + storeArrayIncreaseAmount));
                if(temp == NULL)
                {
                    free(longStringArr);
                    CFDebugLog("CAPunycodeEecodeUnicodeString allcate temp array failed");
                    return NULL;
                }
                longStringArr = temp;
                storeArraySize += storeArrayIncreaseAmount;
            }
            
            longStringArr[currentStoreStringLength++] = '-';
        }
        
        unsigned int alreadyOutputLength = numberOfBasicCodePoint;
        
        while (alreadyOutputLength < sourceStringLength)
        {
            unsigned int min_non_basic_codepoint = overflowValue;
            for (size_t loopIndex = 0;  loopIndex < sourceStringLength;  loopIndex++)
            {
                UTF32Char character = source[loopIndex];
                if (character >= value && character < min_non_basic_codepoint)   // value begin with 0x80
                    min_non_basic_codepoint = source[loopIndex];
            }
            
            if (min_non_basic_codepoint - value > (overflowValue - currentOutputpoints) / (alreadyOutputLength + 1))
            {
                CFDebugLog("CAPunycodeEecodeUnicodeString overflow");
                free(longStringArr);
                return NULL;
            }
            
            // currentOutputpoints is intially zero, actually is the points of output
            currentOutputpoints += (min_non_basic_codepoint - value) * (alreadyOutputLength + 1);
            // intially currentOutputpoints = 0
            value = min_non_basic_codepoint;
            
            for(size_t index = 0; index < sourceStringLength; index++)
            {
                UTF32Char character = source[index];
                
                if(character < value)
                    if (++currentOutputpoints == 0)   // the position of position value added to points is caculated here
                    {
                        CFDebugLog("CAPunycodeEecodeUnicodeString overflow");
                        free(longStringArr);
                        return NULL;
                    }
                
                if (character == value)
                {
                    unsigned int remainingPoints = currentOutputpoints;
                    unsigned timesOfBase = base;
                    
                    LOOP
                    {
                        unsigned int threadhold;
                        if(timesOfBase <= bias + tmin) threadhold = tmin;
                        else if(timesOfBase >= bias + tmax) threadhold = tmax;
                        else threadhold = timesOfBase - bias;     // threadhold = base * (j + 1) - bias (j is current loop index)
                        
                        if(remainingPoints < threadhold)
                            break;
                        
                        UTF32Char codePointForDigit = CAPunycodeEecodeDigit(threadhold + (remainingPoints - threadhold) % (base - threadhold));
                        
                        if(!CAPunycodeIsBasicCodePoint(codePointForDigit))
                        {
                            free(longStringArr);
                            CFDebugLog("CAPunycodeEecodeUnicodeString badcode");
                            return NULL;
                        }
                        
                        if(currentStoreStringLength > storeArraySize)
                        {
                            UTF32Char *temp = realloc(longStringArr, sizeof(UTF32Char) * (storeArraySize + storeArrayIncreaseAmount));
                            if(temp == NULL)
                            {
                                free(longStringArr);
                                CFDebugLog("CAPunycodeEecodeUnicodeString allcate temp array failed");
                                return NULL;
                            }
                            longStringArr = temp;
                            storeArraySize += storeArrayIncreaseAmount;
                        }
                        
                        longStringArr[currentStoreStringLength++] = codePointForDigit;
                        
                        remainingPoints = (remainingPoints - threadhold) / (base - threadhold);
                        timesOfBase += base;
                    }
                    
                    UTF32Char codePointForDigit = CAPunycodeEecodeDigit(remainingPoints);
                    
                    if(!CAPunycodeIsBasicCodePoint(codePointForDigit))
                    {
                        free(longStringArr);
                        CFDebugLog("CAPunycodeEecodeUnicodeString badcode");
                        return NULL;
                    }
                    
                    if(currentStoreStringLength > storeArraySize)
                    {
                        UTF32Char *temp = realloc(longStringArr, sizeof(UTF32Char) * (storeArraySize + storeArrayIncreaseAmount));
                        if(temp == NULL)
                        {
                            free(longStringArr);
                            CFDebugLog("CAPunycodeDecodeASCIIString allcate temp array failed");
                            return NULL;
                        }
                        longStringArr = temp;
                        storeArraySize += storeArrayIncreaseAmount;
                    }
                    
                    longStringArr[currentStoreStringLength++] = codePointForDigit;
                    
                    bias = CAPunycodeBiasAdaptation(currentOutputpoints, alreadyOutputLength+1, alreadyOutputLength == numberOfBasicCodePoint);
                    
                    currentOutputpoints = 0;    // it is cleared here
                    
                    alreadyOutputLength++;
                }
            }
            currentOutputpoints++;
            value++;    // next time value is larger than before
        }
        
        UTF32StringRef result;
        if((result = UTF32StringCreateWithCharacterArray(longStringArr, currentStoreStringLength)) != NULL)
        {
            free(longStringArr);
            return result;
        }
        
        CFDebugLog("CAPunycodeEecodeASCIIString create UTF32String failed");
        
        free(longStringArr);
    }
    return NULL;
}

static const UTF8Char * CAPunycodeStrrchr(const UTF8Char *source, UTF8Char ch)
{
    if(source == NULL) return NULL;
    const UTF8Char *endPoint = source;
    while (*endPoint != '\0') endPoint++;
    while (endPoint >= source)
    {
        if(*endPoint == ch) return endPoint;
        endPoint--;
    }
    return NULL;
}

static size_t CAPunycodeStrlen(const UTF8Char *source)
{
    if(source == NULL) return 0;
    size_t result = 0;
    while (source[result++] != '\0') continue;
    return --result;
}

static size_t CAPunycodeUnicodeStrlen(const UTF32Char *source)
{
    if(source == NULL) return 0;
    size_t result = 0;
    while (source[result++] != '\0') continue;
    return --result;
}

static bool CAPunycodeIsBasicCodePoint(UTF32Char codePoint)
{
    if((codePoint >= 'a' && codePoint <= 'z') ||
       (codePoint >= 'A' && codePoint <= 'Z') ||
       (codePoint >= '0' && codePoint <= '9'))
        return true;
    return false;
}

static unsigned int CAPunycodeDecodeDigit(UTF8Char ch)
{
    if(ch >= 'a' && ch <= 'z') return ch - 'a';
    if(ch >= 'A' && ch <= 'Z') return ch - 'A';
    if(ch >= '0' && ch <= '9') return ch - '0' + 26;
    CFDebugLog("CAPunycodePunycodeDecodeDigit hex:%#X failed, not a valid encoded punycode value", (unsigned int)ch);
    return UINT_MAX;
}

static UTF32Char CAPunycodeEecodeDigit(unsigned int value)
{
    if(value <= 25) return 'a' + value;
    if(value < 36) return '0' + value - 26;
    return (UTF32Char)-1;
}
