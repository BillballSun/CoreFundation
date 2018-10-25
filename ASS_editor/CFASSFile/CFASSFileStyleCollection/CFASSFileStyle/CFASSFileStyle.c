//
//  CFASSFileStyle.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/9.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <wchar.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wctype.h>
#include <limits.h>

#include "CFASSFileStyle.h"
#include "CFASSFile_Private.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"

#define CFASSFileStyleCreateWithStringScanBySteps 0

struct CFASSFileStyle
{
    wchar_t *name,                              // Can't include comma
            *font_name;
    unsigned int font_size;
    CFASSFileStyleColor primary_colour,
                        secondary_colour,
                        outline_colour,
                        back_colour;
    bool blod, italic, underline, strike_out;   // -1 for true, 0 for false
    double scale_x, scale_y;                    // percentage, without digitpoint
    double spacing;                             // extra spacing between characters, in pixels, may be negative and has decimalPoints
    double angle;                               // the origin is defined be alignment
    int border_style;                           // 1 = outline+drop shadow, 3 = opaque box
    unsigned int outline;                       // if BorderStyle is 1, this specify width of ouline around text in pixels
                                                // value may be 0-4
    unsigned int shadow;                        // if BorderStyle is 1, this specify depth of dropping shadow behind text in pixels
                                                // value may be 0-4, Drop shadow is always used in addition to an outline.
                                                // SSA will force an outline of 1 pixel if no outline width is given.
    int alignment;                              // 1 - 9, numberPad
    unsigned int marginL, marginR, marginV;     // margins in pixels
    unsigned int encoding;
};

static bool CFASSFileStyleCreateWithStringIsSkip(wchar_t input);

void CFASSFileStyleMakeChange(CFASSFileStyleRef style, CFASSFileChangeRef change)
{
    if(style == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileStyle %p MakeChange %p", style, change);
    if(change->type & CFASSFileChangeTypeFontSize)
    {
        if(change->fontSize.byPercentage)
            style->font_size *= change->fontSize.percentage;
        else
            style->font_size = change->fontSize.fontSize;
    }
    if(change->type & CFASSFileChangeTypeFontName)
    {
        wchar_t *dumped = CF_Dump_wchar_string(change->fontName.fontName);
        if(dumped != NULL)
        {
            free(style->font_name);
            style->font_name = dumped;
        }
    }
    if(change->type & CFASSFileChangeTypePrimaryColor)
    {
        style->primary_colour = CFASSFileStyleColorMake(change->primaryColor.alpha, change->primaryColor.blue, change->primaryColor.green, change->primaryColor.red);
    }
    if(change->type & CFASSFileChangeTypeOutlineColor)
    {
        style->outline_colour = CFASSFileStyleColorMake(change->outlineColor.alpha, change->outlineColor.blue, change->outlineColor.green, change->outlineColor.red);
    }
    if(change->type & CFASSFileChangeTypeBlod)
    {
        style->blod = change->blod.isBlod;
    }
    if(change->type & CFASSFileChangeTypeBorderStyle)
    {
        style->border_style = change->borderStyle.borderStyle;
    }
    if(change->type & CFASSFileChangeTypeShadow)
    {
        if(change->shadow.forceBorderStyle)
            style->border_style = 1;
        if(style->border_style == 1)
            style->shadow = change->shadow.pixel;
    }
    if(change->type & CFASSFileChangeTypeOutline)
    {
        if(change->outline.forceBorderStyle)
            style->border_style = 1;
        if(style->border_style == 1)
            style->outline = change->outline.pixel;
    }
}

CFASSFileStyleRef CFASSFileStyleCopy(CFASSFileStyleRef style)
{
    CFASSFileStyleRef result;
    if((result = malloc(sizeof(struct CFASSFileStyle))) != NULL)
    {
        if((result->name = CF_Dump_wchar_string(style->name))!=NULL)
        {
            if((result->font_name = CF_Dump_wchar_string(style->font_name))!=NULL)
            {
                result->font_size = style->font_size;
                result->primary_colour = style->primary_colour;
                result->secondary_colour = style->secondary_colour;
                result->outline_colour = style->outline_colour;
                result->blod = style->blod;
                result->italic = style->italic;
                result->underline = style->underline;
                result->strike_out = style->strike_out;
                result->scale_x = style->scale_x;
                result->scale_y = style->scale_y;
                result->spacing = style->spacing;
                result->angle = style->angle;
                result->border_style = style->border_style;
                result->outline = style->outline;
                result->shadow = style->shadow;
                result->alignment = style->alignment;
                result->marginL = style->marginL;
                result->marginR = style->marginR;
                result->marginV = style->marginV;
                result->encoding = style->encoding;
                
                return result;
            }
            free(result->name);
        }
        free(result);
    }
    return NULL;
}

void CFASSFileStyleDestory(CFASSFileStyleRef style)
{
    if(style==NULL) return;
    free(style->name);
    free(style->font_name);
    free(style);
}

CFASSFileStyleRef CFASSFileStyleCreate(wchar_t *name,
                                       wchar_t *font_name, unsigned int font_size,
                                       CFASSFileStyleColor primary_colour,
                                       CFASSFileStyleColor secondary_colour,
                                       CFASSFileStyleColor outline_colour,
                                       bool blod, bool italic, bool underline, bool strike_out,
                                       double scale_x, double scale_y,
                                       double spacing,
                                       double angle,
                                       int border_style,
                                       unsigned int outline, unsigned int shadow,
                                       int alignment,
                                       unsigned int marginL, unsigned int marginR, unsigned marginV,
                                       unsigned encoding)
{
    if(name == NULL || font_name == NULL || font_size == 0) return NULL;
    if(border_style!=1 && border_style!=3) return NULL;
    if(outline>4) return NULL;
    if((border_style!=1 && shadow!=0) || shadow>4 || (shadow!=0 && outline==0)) return NULL;
    if(alignment<1 || alignment>9) return NULL;
    
    CFASSFileStyleRef result;
    
    if((result = malloc(sizeof(struct CFASSFileStyle)))!=NULL)
    {
        if((result->name = CF_Dump_wchar_string(name))!=NULL)
        {
            if((result->font_name = CF_Dump_wchar_string(font_name))!=NULL)
            {
                result->font_size = font_size;
                result->primary_colour = primary_colour;
                result->secondary_colour = secondary_colour;
                result->outline_colour = outline_colour;
                result->blod = blod;
                result->italic = italic;
                result->underline = underline;
                result->strike_out = strike_out;
                result->scale_x = scale_x;
                result->scale_y = scale_y;
                result->spacing = spacing;
                result->angle = angle;
                result->border_style = border_style;
                result->outline = outline;
                result->shadow = shadow;
                result->alignment = alignment;
                result->marginL = marginL;
                result->marginR = marginR;
                result->marginV = marginV;
                result->encoding = encoding;
                
                return result;
            }
            free(result->name);
        }
        free(result);
    }
    
    return NULL;
}

wchar_t *CFASSFileStyleAllocateFileContent(CFASSFileStyleRef style)
{
    FILE *fp;
    if((fp = tmpfile())==NULL) return NULL;
    
    int stringLength =
    fwprintf(fp,
             L"Style: %ls,%ls,%u,"                  /* font */
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"     /* colour */
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
             L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
             L"%d,%d,%d,%d,"                        /* blod, italic, underline, strike_out */
             L"%g,%g,"                              /* scale_x, scale_y */
             L"%g,"                                 /* spacing */
             L"%g,"                                 /* angle */
             L"%d,"                                 /* border_style */
             L"%u,%u,"                              /* outline, shadow */
             L"%d,"                                 /* alignment */
             L"%u,%u,%u,"                           /* margins */
             L"%u\n",                              /* encoding */
             style->name, style->font_name, style->font_size,
             (unsigned int)style->primary_colour.alpha, (unsigned int)style->primary_colour.blue, (unsigned int)style->primary_colour.green, (unsigned int)style->primary_colour.red,
             (unsigned int)style->secondary_colour.alpha, (unsigned int)style->secondary_colour.blue, (unsigned int)style->secondary_colour.green, (unsigned int)style->secondary_colour.red,
             (unsigned int)style->outline_colour.alpha, (unsigned int)style->outline_colour.blue, (unsigned int)style->outline_colour.green, (unsigned int)style->outline_colour.red,
             (unsigned int)style->back_colour.alpha, (unsigned int)style->back_colour.blue, (unsigned int)style->back_colour.green, (unsigned int)style->back_colour.red,
             style->blod?-1:0,style->italic?-1:0,style->underline?-1:0,style->strike_out?-1:0,
             style->scale_x, style->scale_y,
             style->spacing,
             style->angle,
             style->border_style,
             style->outline, style->shadow,
             style->alignment,
             style->marginL, style->marginR, style->marginV,
             style->encoding);
    
    wchar_t *result;
    if((result=malloc(sizeof(wchar_t)*(stringLength+1)))!=NULL)
    {
        swprintf(result, stringLength+1,
                 L"Style: %ls,%ls,%u,"                  /* font */
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"         /* colour */
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
                 L"&H%2.2hX%2.2hX%2.2hX%2.2hX,"
                 L"%d,%d,%d,%d,"                        /* blod, italic, underline, strike_out */
                 L"%g,%g,"                              /* scale_x, scale_y */
                 L"%g,"                                 /* spacing */
                 L"%g,"                                 /* angle */
                 L"%d,"                                 /* border_style */
                 L"%u,%u,"                              /* outline, shadow */
                 L"%d,"                                 /* alignment */
                 L"%u,%u,%u,"                           /* margins */
                 L"%u\n",                               /* encoding */
                 style->name, style->font_name, style->font_size,
                 (unsigned int)style->primary_colour.alpha, (unsigned int)style->primary_colour.blue, (unsigned int)style->primary_colour.green, (unsigned int)style->primary_colour.red,
                 (unsigned int)style->secondary_colour.alpha, (unsigned int)style->secondary_colour.blue, (unsigned int)style->secondary_colour.green, (unsigned int)style->secondary_colour.red,
                 (unsigned int)style->outline_colour.alpha, (unsigned int)style->outline_colour.blue, (unsigned int)style->outline_colour.green, (unsigned int)style->outline_colour.red,
                 (unsigned int)style->back_colour.alpha, (unsigned int)style->back_colour.blue, (unsigned int)style->back_colour.green, (unsigned int)style->back_colour.red,
                 style->blod?-1:0,style->italic?-1:0,style->underline?-1:0,style->strike_out?-1:0,
                 style->scale_x, style->scale_y,
                 style->spacing,
                 style->angle,
                 style->border_style,
                 style->outline, style->shadow,
                 style->alignment,
                 style->marginL, style->marginR, style->marginV,
                 style->encoding);
        return result;
    }
    fclose(fp);
    return NULL;
}

CFASSFileStyleRef CFASSFileStyleCreateWithString(const wchar_t *content)
{
    const wchar_t *beginPoint = content,
                  *endPoint = content;          /* points to L'\n' */
    while(*endPoint!=L'\n') endPoint++;
    int scanedAmount = (int)wcslen(L"Style:");
    swscanf(beginPoint, L"Style:%n", &scanedAmount);
    if(scanedAmount == wcslen(L"Style:"))
    {
        beginPoint+=scanedAmount;
        CFASSFileStyleRef result;
        if((result = malloc(sizeof(struct CFASSFileStyle))) != NULL)
        {
            if((result->name = CF_allocate_wchar_string_with_endPoint_skip_terminate(beginPoint, endPoint - 1, CFASSFileStyleCreateWithStringIsSkip, NULL)) != NULL)
            {
                while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                if(*beginPoint == L',')
                {
                    beginPoint++;
                    if((result->font_name = CF_allocate_wchar_string_with_endPoint_skip_terminate(beginPoint, endPoint - 1, CFASSFileStyleCreateWithStringIsSkip, NULL)) != NULL)
                    {
                        #if CFASSFileStyleCreateWithStringScanBySteps
                        
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->font_size, &scanedAmount) != 1) goto LABEL_1;
                        if(result->font_size == 0) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        unsigned short alpha, blue, green, red;
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                                                        &blue,
                                                                        &green,
                                                                        &red,
                                                                        &scanedAmount) != 4) goto LABEL_1;
                        if(alpha>0xFF || blue>0xFF || green>0xFF || red>0xFF)
                            goto LABEL_1;
                        
                        result->primary_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                                                        &blue,
                                                                        &green,
                                                                        &red,
                                                                        &scanedAmount) != 4) goto LABEL_1;
                        
                        if(alpha>0xFF || blue>0xFF || green>0xFF || red>0xFF)
                            goto LABEL_1;
                        
                        result->secondary_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                   &blue,
                                   &green,
                                   &red,
                                   &scanedAmount) != 4) goto LABEL_1;
                        
                        if(alpha>0xFF || blue>0xFF || green>0xFF || red>0xFF)
                            goto LABEL_1;
                        
                        result->outline_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"&H%2hX%2hX%2hX%2hX%n", &alpha,
                                   &blue,
                                   &green,
                                   &red,
                                   &scanedAmount) != 4) goto LABEL_1;
                        
                        if(alpha>0xFF || blue>0xFF || green>0xFF || red>0xFF)
                            goto LABEL_1;
                        
                        result->back_colour = CFASSFileStyleColorMake(alpha, blue, green, red);
                        
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        int temp;
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) goto LABEL_1;
                        if(temp == -1)
                            result->blod = true;
                        else if(temp == 0)
                            result->blod = false;
                        else goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) goto LABEL_1;
                        if(temp == -1)
                            result->italic = true;
                        else if(temp == 0)
                            result->italic = false;
                        else goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) goto LABEL_1;
                        if(temp == -1)
                            result->underline = true;
                        else if(temp == 0)
                            result->underline = false;
                        else goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%d%n", &temp, &scanedAmount) != 1) goto LABEL_1;
                        if(temp == -1)
                            result->strike_out = true;
                        else if(temp == 0)
                            result->strike_out = false;
                        else goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->scale_x, &scanedAmount) != 1) goto LABEL_1;
                        if(result->scale_x<0.0) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->scale_y, &scanedAmount) != 1) goto LABEL_1;
                        if(result->scale_y<0.0) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->spacing, &scanedAmount) != 1) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%lf%n", &result->angle, &scanedAmount) != 1) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%d%n", &result->border_style, &scanedAmount) != 1) goto LABEL_1;
                        if(result->border_style!=1 && result->border_style!=3) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->outline, &scanedAmount) != 1) goto LABEL_1;
                        if((result->border_style!=1 && result->outline!=0) || result->outline>4) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->shadow, &scanedAmount) != 1) goto LABEL_1;
                        if((result->border_style!=1 && result->shadow!=0) || result->shadow>4 || (result->shadow!=0 && result->outline==0)) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%d%n", &result->alignment, &scanedAmount) != 1) goto LABEL_1;
                        if(result->alignment<1 || result->alignment>9) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->marginL, &scanedAmount) != 1) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->marginR, &scanedAmount) != 1) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->marginV, &scanedAmount) != 1) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        if(swscanf(beginPoint, L"%u%n", &result->encoding, &scanedAmount) != 1) goto LABEL_1;
                        beginPoint += scanedAmount;
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',') beginPoint++;
                        else goto LABEL_1;
                        
                        return result;
                        
                        LABEL_1:free(result->font_name);
                        
                        #else
                        
                        while(*beginPoint!=L',' && beginPoint<endPoint) beginPoint++;
                        if(*beginPoint == L',')
                        {
                            beginPoint++;
                            
                            int blod, italic, underline, strike_out;
                            
                            unsigned short primary_alpha, primary_blue, primary_green, primary_red;
                            unsigned short secondary_alpha, secondary_blue, secondary_green, secondary_red;
                            unsigned short outline_alpha, outline_blue, outline_green, outline_red;
                            unsigned short back_alpha, back_blue, back_green, back_red;
                            
                            if(swscanf(beginPoint,
                                       L"%u,"               /* font_size */
                                       L"&H%2hX%2hX%2hX%2hX,&H%2hX%2hX%2hX%2hX,&H%2hX%2hX%2hX%2hX,&H%2hX%2hX%2hX%2hX,"
                                       L"%d,%d,%d,%d,"      /* blod, italic, underline, strike_out */
                                       L"%lf,%lf,"          /* scale_x, scale_y */
                                       L"%lf,"               /* spacing */
                                       L"%lf,"              /* angle */
                                       L"%d,"               /* boarder_style */
                                       L"%u,%u,"            /* outline, shadow */
                                       L"%d,"               /* alignment */
                                       L"%u,%u,%u,"         /* margins */
                                       L"%u",               /* encoding */
                                       &result->font_size,
                                       &primary_alpha, &primary_blue, &primary_green, &primary_red,
                                       &secondary_alpha, &secondary_blue, &secondary_green, &secondary_red,
                                       &outline_alpha, &outline_blue, &outline_green, &outline_red,
                                       &back_alpha, &back_blue, &back_green, &back_red,
                                       &blod, &italic, &underline, &strike_out,
                                       &result->scale_x, &result->scale_y,
                                       &result->spacing,
                                       &result->angle,
                                       &result->border_style,
                                       &result->outline, &result->shadow,
                                       &result->alignment,
                                       &result->marginL, &result->marginR, &result->marginV,
                                       &result->encoding,
                                       &scanedAmount) == 33)
                            {
                                bool isFormatCorrect = true;
                                
                                if(result->font_size == 0) isFormatCorrect = false;
                                
                                if(primary_alpha>0xFF || primary_blue>0xFF || primary_green>0xFF || primary_red>0xFF) isFormatCorrect = false;
                                else result->primary_colour = CFASSFileStyleColorMake(primary_alpha, primary_blue, primary_green, primary_red);
                                
                                if(secondary_alpha>0xFF || secondary_blue>0xFF || secondary_green>0xFF || secondary_red>0xFF) isFormatCorrect = false;
                                else result->secondary_colour = CFASSFileStyleColorMake(secondary_alpha, secondary_blue, secondary_green, secondary_red);
                                
                                if(outline_alpha>0xFF || outline_blue>0xFF || outline_green>0xFF || outline_red>0xFF) isFormatCorrect = false;
                                else result->outline_colour = CFASSFileStyleColorMake(outline_alpha, outline_blue, outline_green, outline_red);
                                
                                if(back_alpha>0xFF || back_blue>0xFF || back_green>0xFF || back_red>0xFF) isFormatCorrect = false;
                                else result->back_colour = CFASSFileStyleColorMake(back_alpha, back_blue, back_green, back_red);
                                
                                if(blod == -1 || blod == 0) result->blod = blod;
                                else isFormatCorrect = false;
                                if(italic == -1 || italic == 0) result->italic = italic;
                                else isFormatCorrect = false;
                                if(underline == -1 || underline == 0) result->underline = underline;
                                else isFormatCorrect = false;
                                if(strike_out == -1 || strike_out == 0) result->strike_out = strike_out;
                                else isFormatCorrect = false;
                                if(result->border_style!=1 && result->border_style!=3)
                                    isFormatCorrect = false;
                                if(result->outline>4) isFormatCorrect = false;
                                if((result->border_style!=1 && result->shadow!=0) || result->shadow>4 || (result->shadow!=0 && result->outline==0))
                                    isFormatCorrect = false;
                                if(result->alignment<1 || result->alignment>9)
                                    isFormatCorrect = false;
                                
                                if(isFormatCorrect) return result;
                            }
                        }
                        free(result->font_name);
                        
                        #endif

                    }
                }
                free(result->name);
            }
            free(result);
        }
    }
    return NULL;
}

static bool CFASSFileStyleCreateWithStringIsSkip(wchar_t input)
{
    if(input==L',')
        return true;
    else
        return false;
}

CFASSFileStyleColor CFASSFileStyleColorMake(unsigned char alpha,
                                            unsigned char blue,
                                            unsigned char green,
                                            unsigned char red)
{
    return (CFASSFileStyleColor){alpha, blue, green, red};
}
