//
//  CFASSFileScriptInfo.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/8.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>

#include "CFASSFile.h"
#include "CFASSFile_Private.h"
#include "CFASSFileScriptInfo.h"
#include "CFASSFileScriptInfo_Private.h"
#include "CFUnicodeStringArray.h"
#include "CFUseTool.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"

#define CFASSFileScriptInfoSearchEachContentName(WSTR) (L"\n" WSTR L":")

#define CFASSFileScriptInfoFileContentSearchInOrder 1

struct CFASSFileScriptInfo {
    CFUnicodeStringArrayRef comment;
    wchar_t *title,                 // This is a description of the script
            *original_script,       // The original author(s) of the script
                                    // If the original author(s) did not provide then <unknown> is auto added
            *original_translation,  // (optional) The original translator of the dialogue
            *original_editing,      // (optional) The original script editor(s)
            *original_timing,       // (optional) whoever timed the original script
            *synch_point,           // (optional) Description of where in the video the script should begin playback
            *script_updated_by,     // (optional) Names of any other subtitling groups who edited the original script
            *update_details,        // (optional) The details of any updates to the original script
            *script_type;           // This is the SSA script format Version eg. V4.00+
    bool is_collisions_normal;       // Normal Reverse
    unsigned int play_res_x, play_res_y;
    wchar_t *play_depth;            // (optional)This is the colour depth used by the script's author(s) when playing the script
    double timer;                   // This is the Timer Speed for the script, as a percentage
                                    // eg. 100.0000 is eactly 100%. It has four digits following the decimal point.
    unsigned int wrap_style;        // (optional)0 - 4
    CFASSFileRef registeredFile;    // don't have ownership
};

void CFASSFileScriptInfoMakeChange(CFASSFileScriptInfoRef scriptInfo, CFASSFileChangeRef change)
{
    if(scriptInfo == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileScriptInfo %p MakeChange %p", scriptInfo, change);
    if(change->type & CFASSFileChangeTypeResolution)
    {
        scriptInfo->play_res_x = change->resolution.newSize.x;
        scriptInfo->play_res_y = change->resolution.newSize.y;
    }
}

int CFASSFileScriptInfoRegisterAssociationwithFile(CFASSFileScriptInfoRef scriptInfo, CFASSFileRef assFile)
{
    if(scriptInfo->registeredFile == NULL)
    {
        scriptInfo->registeredFile = assFile;
        return 0;
    }
    else
        return -1;
}

wchar_t *CFASSFileScriptInfoAllocateFileContent(CFASSFileScriptInfoRef scriptInfo)
{
    size_t commentAmount = CFUnicodeStringArrayGetLength(scriptInfo->comment),
    scriptStringLength = 0;
    int temp;
    
    scriptStringLength += wcslen(L"[Script Info]\n");
    
    for(size_t count = 1; count<=commentAmount; count++)
        scriptStringLength += wcslen(CFUnicodeStringArrayGetStringAtIndex(scriptInfo->comment, count-1))+2;
    
    FILE *fp;
    if((fp=tmpfile()) == NULL) return NULL;
    
    temp =
    fwprintf(fp, L"Title:%ls\n"
                 L"Original Script:%ls\n",
                scriptInfo->title==NULL?L"":scriptInfo->title,
                scriptInfo->original_script==NULL?L"":scriptInfo->original_script);
    if(temp<0) {fclose(fp); return NULL;}
    else scriptStringLength+=temp;
    
    if(scriptInfo->original_translation!=NULL) {
        temp = fwprintf(fp, L"Original Translation:%ls\n", scriptInfo->original_translation);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    if(scriptInfo->original_editing!=NULL) {
        temp = fwprintf(fp, L"Original Editing:%ls\n", scriptInfo->original_editing);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    if(scriptInfo->original_timing!=NULL) {
        temp = fwprintf(fp, L"Original Timing:%ls\n", scriptInfo->original_timing);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    if(scriptInfo->synch_point!=NULL) {
        temp = fwprintf(fp, L"Synch Point:%ls\n", scriptInfo->synch_point);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    if(scriptInfo->script_updated_by!=NULL) {
        temp = fwprintf(fp, L"Script Updated By:%ls\n", scriptInfo->script_updated_by);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    if(scriptInfo->update_details!=NULL) {
        temp = fwprintf(fp, L"Update Details:%ls\n", scriptInfo->update_details);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    if(scriptInfo->script_type!=NULL) {
        temp = fwprintf(fp, L"ScriptType:%ls\n", scriptInfo->script_type);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    
    if(scriptInfo->is_collisions_normal)
        scriptStringLength+=wcslen(L"Collisions:Normal\n");
    else
        scriptStringLength+=wcslen(L"Collisions:Reversed\n");
    
    temp =
    fwprintf(fp, L"PlayResX:%u\n"
                 L"PlayResY:%u\n",
                 scriptInfo->play_res_x,
                 scriptInfo->play_res_y);
    if(temp<0) {fclose(fp); return NULL;}
    else scriptStringLength+=temp;
    
    if(scriptInfo->play_depth!=NULL) {
        temp = fwprintf(fp, L"PlayDepth:%ls\n", scriptInfo->play_depth);
        if(temp<0) {fclose(fp); return NULL;}
        else scriptStringLength+=temp;
    }
    
    temp =
    fwprintf(fp, L"Timer:%.4f\n"
                 L"WrapStyle:%u\n",
                 scriptInfo->timer,
                 scriptInfo->wrap_style);
    if(temp<0) {fclose(fp); return NULL;}
    else scriptStringLength+=temp;
    
    wchar_t *result;
    if((result = malloc(sizeof(wchar_t)*(scriptStringLength+1))) == NULL)
        {fclose(fp); return NULL;}
    
    wchar_t *writingPoint = result;
    
    temp = swprintf(writingPoint, scriptStringLength+1, L"[Script Info]\n");
    writingPoint+=temp; scriptStringLength-=temp;
    
    for(size_t count = 1; count<=commentAmount; count++)
    {
        temp = swprintf(writingPoint, scriptStringLength+1, L";%ls\n", CFUnicodeStringArrayGetStringAtIndex(scriptInfo->comment, count-1));
        writingPoint+=temp; scriptStringLength-=temp;
    }
    
    temp =
    swprintf(writingPoint, scriptStringLength+1,
             L"Title:%ls\n"
             L"Original Script:%ls\n",
             scriptInfo->title==NULL?L"":scriptInfo->title,
             scriptInfo->original_script==NULL?L"":scriptInfo->original_script);
    writingPoint+=temp; scriptStringLength-=temp;
    
    if(scriptInfo->original_translation!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"Original Translation:%ls\n", scriptInfo->original_translation);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    if(scriptInfo->original_editing!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"Original Editing:%ls\n", scriptInfo->original_editing);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    if(scriptInfo->original_timing!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"Original Timing:%ls\n", scriptInfo->original_timing);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    if(scriptInfo->synch_point!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"Synch Point:%ls\n", scriptInfo->synch_point);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    if(scriptInfo->script_updated_by!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"Script Updated By:%ls\n", scriptInfo->script_updated_by);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    if(scriptInfo->update_details!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"Update Details:%ls\n", scriptInfo->update_details);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    if(scriptInfo->script_type!=NULL) {
        temp =
        swprintf(writingPoint, scriptStringLength+1,
                 L"ScriptType:%ls\n", scriptInfo->script_type);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    
    if(scriptInfo->is_collisions_normal)
    {
        temp =
        swprintf(writingPoint, scriptStringLength+1, L"Collisions:Normal\n");
        writingPoint+=temp; scriptStringLength-=temp;
    }
    else
    {
        temp =
        swprintf(writingPoint, scriptStringLength+1, L"Collisions:Reversed\n");
        writingPoint+=temp; scriptStringLength-=temp;
    }
    
    temp =
    swprintf(writingPoint, scriptStringLength+1,
             L"PlayResX:%u\n"
             L"PlayResY:%u\n",
             scriptInfo->play_res_x,
             scriptInfo->play_res_y);
    writingPoint+=temp; scriptStringLength-=temp;
    
    if(scriptInfo->play_depth!=NULL) {
        temp = swprintf(writingPoint, scriptStringLength+1,
                 L"PlayDepth:%ls\n", scriptInfo->play_depth);
        writingPoint+=temp; scriptStringLength-=temp;
    }
    temp =
    swprintf(writingPoint, scriptStringLength+1,
             L"Timer:%.4f\n"
             L"WrapStyle:%u\n",
             scriptInfo->timer,
             scriptInfo->wrap_style);
    // writingPoint+=temp; scriptStringLength-=temp;
    
    fclose(fp);
    return result;
}

static wchar_t *CFASSFileScriptInfoAllocateEachContent(const wchar_t *name,         /* sample: L"Tiltle", without comma */
                                                       const wchar_t *beginPoint,
                                                       const wchar_t *endPoint,
                                                       size_t *shouldSkip);

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateEssential(wchar_t *title,
                                                          wchar_t *original_script,
                                                          bool is_collisions_normal,
                                                          unsigned int play_res_x,
                                                          unsigned int play_res_y,
                                                          double timePercentage)
{
    CFASSFileScriptInfoRef result;
    if((result = malloc(sizeof(struct CFASSFileScriptInfo))) != NULL)
    {
        if((result->comment = CFUnicodeStringArrayCreateEmpty()) != NULL)
        {
            result->title = NULL; result->original_script = NULL; result->original_translation = NULL;
            result->original_editing = NULL; result->original_timing = NULL; result->synch_point = NULL;
            result->script_updated_by = NULL; result->update_details = NULL; result->script_type = NULL;
            result->play_depth = NULL;
            if(title==NULL || (result->title=CF_Dump_wchar_string(title))!=NULL){
                if(original_script==NULL || (result->original_script=CF_Dump_wchar_string(original_script))!=NULL){
                    result->is_collisions_normal = is_collisions_normal;
                    result->play_res_x = play_res_x;
                    result->play_res_y = play_res_y;
                    result->timer = timePercentage;
                    result->wrap_style = 0u;
                    result->registeredFile = NULL;
                    return result;
                }
                free(result->title);
            }
            CFUnicodeStringArrayDestory(result->comment);
        }
        free(result);
    }
    return NULL;
}

CFASSFileScriptInfoRef CFASSFileScriptInfoCreateWithUnicodeFileContent(const wchar_t *content)
{
    CFASSFileScriptInfoRef result;
    if((result = malloc(sizeof(struct CFASSFileScriptInfo))) == NULL)
        return NULL;
    
    if((result->comment = CFUnicodeStringArrayCreateEmpty()) == NULL)
    {
        free(result);
        return NULL;
    }
    result->registeredFile = NULL;
    
    const wchar_t *beginPoint = content;
    
    int scanAmount = 0;
    int compareResult = wmemcmp(beginPoint, L"[Script Info]\n", wcslen(L"[Script Info]\n"));
    if(compareResult!=0)
    {
        CFUnicodeStringArrayDestory(result->comment);
        free(result);
        return NULL;
    }
    beginPoint += wcslen(L"[Script Info]\n");
    
    const wchar_t *endPoint;
    if((endPoint = wcsstr(beginPoint, L"\n[V4+ Styles]"))==NULL)
    {
        CFUnicodeStringArrayDestory(result->comment);
        free(result);
        return NULL;
    }
    
    bool encounterEndChar; size_t length;
    while(*beginPoint == L';')
    {
        length = CFUnicodeStringArrayAddStringWithEndChar(result->comment, beginPoint+1, L'\n', &encounterEndChar);
        if(!encounterEndChar)
        {
            CFUnicodeStringArrayDestory(result->comment);
            free(result);
            return NULL;
        }
        beginPoint += length+2;
    }
    beginPoint--;
    
    #if CFASSFileScriptInfoFileContentSearchInOrder
    
    size_t skipAmount;
    
    result->title =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Title"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->original_script =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Script"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    result->original_translation =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Translation"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->original_editing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Editing"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->original_timing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Timing"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->synch_point =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Synch Point"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->script_updated_by =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Script Updated By"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->update_details =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Update Details"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    result->script_type =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"ScriptType"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    const wchar_t *dataPoint;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"Collisions"),
                                          endPoint))!=NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"Collisions"));
        scanAmount = 0;
        swscanf(dataPoint, L"Normal%n", &scanAmount);
        if(scanAmount == wcslen(L"Normal"))
            result->is_collisions_normal = true;
        else
        {
            scanAmount = 0;
            swscanf(dataPoint, L"Reverse%n", &scanAmount);
            if(scanAmount == wcslen(L"Reverse"))
                result->is_collisions_normal = false;
            else
                result->is_collisions_normal = true;
        }
        do beginPoint++; while(*beginPoint!=L'\n');
    }
    else
        result->is_collisions_normal = true;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"PlayResX"),
                                          endPoint)) != NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"PlayResX"));
        if(swscanf(dataPoint, L"%u", &result->play_res_x) != 1)
            result->play_res_x = 0u;
        do beginPoint++; while(*beginPoint!=L'\n');
    }
    else
        result->play_res_x = 0u;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"PlayResY"),
                                          endPoint)) != NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"PlayResY"));
        if(swscanf(dataPoint, L"%u", &result->play_res_y) != 1)
            result->play_res_y = 0u;
        do beginPoint++; while(*beginPoint!=L'\n');
    }
    else
        result->play_res_y = 0u;
    
    result->play_depth =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayDepth"),
                                           beginPoint,
                                           endPoint,
                                           &skipAmount);
    beginPoint+=skipAmount;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"Timer"),
                                          endPoint)) != NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"Timer"));
        if(swscanf(dataPoint, L"%lf", &result->timer) != 1)
            result->timer = 100.0;
        do beginPoint++; while(*beginPoint!=L'\n');
    }
    else
        result->timer = 100.0;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"WrapStyle"),
                                          endPoint))!=NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"WrapStyle"));
        if(swscanf(dataPoint, L"%u", &result->wrap_style)!=1 || result->wrap_style > 4)
            result->wrap_style = 0u;
    }
    else
        result->wrap_style = 0u;
    
    #else
    
    result->title =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Title"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_script =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Script"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_translation =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Translation"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_editing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Editing"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->original_timing =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Original Timing"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->synch_point =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Synch Point"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->script_updated_by =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Script Updated By"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->update_details =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"Update Details"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    result->script_type =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"ScriptType"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    
    const wchar_t *dataPoint;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"Collisions"),
                                          endPoint))!=NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"Collisions"));
        scanAmount = 0;
        swscanf(dataPoint, L"Normal%n", &scanAmount);
        if(scanAmount == wcslen(L"Normal"))
            result->is_collisions_normal = true;
        else
        {
            scanAmount = 0;
            swscanf(dataPoint, L"Reverse%n", &scanAmount);
            if(scanAmount == wcslen(L"Reverse"))
                result->is_collisions_normal = false;
            else
                result->is_collisions_normal = true;
        }
    }
    else
        result->is_collisions_normal = true;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"PlaResX"),
                                          endPoint)) != NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"PlaResX"));
        if(swscanf(dataPoint, L"%u", &result->play_res_x) != 1)
            result->play_res_x = 0u;
    }
    else
        result->play_res_x = 0u;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"PlaResY"),
                                          endPoint)) != NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"PlaResY"));
        if(swscanf(dataPoint, L"%u", &result->play_res_y) != 1)
            result->play_res_y = 0u;
    }
    else
        result->play_res_y = 0u;
    
    result->play_depth =
    CFASSFileScriptInfoAllocateEachContent(CFASSFileScriptInfoSearchEachContentName(L"PlayDepth"),
                                           beginPoint,
                                           endPoint,
                                           NULL);
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"Timer"),
                                          endPoint)) != NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"Timer"));
        if(swscanf(dataPoint, L"%lf", &result->timer) != 1)
            result->timer = 100.0;
    }
    else
        result->timer = 100.0;
    
    if((dataPoint = CF_wcsstr_with_end_point(beginPoint,
                                          CFASSFileScriptInfoSearchEachContentName(L"WrapStyle"),
                                          endPoint))!=NULL)
    {
        dataPoint += wcslen(CFASSFileScriptInfoSearchEachContentName(L"WrapStyle"));
        if(swscanf(dataPoint, L"%u", &result->wrap_style)!=1 || result->wrap_style > 4)
            result->wrap_style = 0u;
    }
    else
        result->wrap_style = 0u;
    
    #endif
    
    return result;
}

static wchar_t *CFASSFileScriptInfoAllocateEachContent(const wchar_t *name,
                                                       const wchar_t *beginPoint,
                                                       const wchar_t *endPoint,
                                                       size_t *shouldSkip)
{
    if(shouldSkip!=NULL) *shouldSkip = 0;
    const wchar_t *searchPoint;
    if((searchPoint = CF_wcsstr_with_end_point(beginPoint, name, endPoint))!=NULL)
    {
        searchPoint += wcslen(name);
        (*shouldSkip) += wcslen(name);
        const wchar_t *contentEndPoint = CF_wcsstr_with_end_point(searchPoint, L"\n", endPoint);
        if(contentEndPoint!=NULL)
        {
            ptrdiff_t length = contentEndPoint - searchPoint;
            if(shouldSkip!=NULL) (*shouldSkip) += length;
            if(length!=0)
            {
                wchar_t *result = malloc(sizeof(wchar_t)*(length+1));
                if(result!=NULL)
                {
                    wmemcpy(result, searchPoint, length);
                    result[length] = L'\0';
                    return result;
                }
            }
        }
    }
    return NULL;
}

CFASSFileScriptInfoRef CFASSFileScriptInfoCopy(CFASSFileScriptInfoRef scriptInfo)
{
    /* the fucking loop makes my code seems ugly */
    CFASSFileScriptInfoRef duplicated = malloc(sizeof(struct CFASSFileScriptInfo));
    if(duplicated==NULL) return NULL;
    if((duplicated->comment = CFUnicodeStringArrayCopy(scriptInfo->comment)) != NULL) {
        if(scriptInfo->title == NULL || (duplicated->title = CF_Dump_wchar_string(scriptInfo->title)) != NULL){
            if(scriptInfo->original_script == NULL || (duplicated->original_script = CF_Dump_wchar_string(scriptInfo->original_script)) != NULL){
                if(scriptInfo->original_translation == NULL || (duplicated->original_translation = CF_Dump_wchar_string(scriptInfo->original_translation)) != NULL){
                    if(scriptInfo->original_editing == NULL || (duplicated->original_editing = CF_Dump_wchar_string(scriptInfo->original_editing)) != NULL){
                        if(scriptInfo->original_timing == NULL || (duplicated->original_timing = CF_Dump_wchar_string(scriptInfo->original_timing)) != NULL){
                            if(scriptInfo->synch_point == NULL || (duplicated->synch_point = CF_Dump_wchar_string(scriptInfo->synch_point)) != NULL){
                                if(scriptInfo->script_updated_by == NULL || (duplicated->script_updated_by = CF_Dump_wchar_string(scriptInfo->script_updated_by)) != NULL){
                                    if(scriptInfo->update_details == NULL || (duplicated->update_details = CF_Dump_wchar_string(scriptInfo->update_details)) != NULL){
                                        if(scriptInfo->script_type == NULL || (duplicated->script_type = CF_Dump_wchar_string(scriptInfo->script_type)) != NULL){
                                            if(scriptInfo->play_depth == NULL || (duplicated->play_depth = CF_Dump_wchar_string(scriptInfo->play_depth)) != NULL){
                                                duplicated->registeredFile = NULL;
                                                return duplicated;
                                            }
                                            free(duplicated->script_type);
                                        }
                                        free(duplicated->update_details);
                                    }
                                    free(duplicated->script_updated_by);
                                }
                                free(duplicated->synch_point);
                            }
                            free(duplicated->original_timing);
                        }
                        free(duplicated->original_editing);
                    }
                    free(duplicated->original_translation);
                }
                free(duplicated->original_script);
            }
            free(duplicated->title);
        }
        CFUnicodeStringArrayDestory(duplicated->comment);
    }
    free(duplicated);
    return NULL;
}

void CFASSFileScriptInfoDestory(CFASSFileScriptInfoRef scriptInfo)
{
    if(scriptInfo == NULL) return;
    CFUnicodeStringArrayDestory(scriptInfo->comment);
    free(scriptInfo->title);
    free(scriptInfo->original_script);
    free(scriptInfo->original_translation);
    free(scriptInfo->original_editing);
    free(scriptInfo->original_timing);
    free(scriptInfo->synch_point);
    free(scriptInfo->script_updated_by);
    free(scriptInfo->update_details);
    free(scriptInfo->script_type);
    free(scriptInfo->play_depth);
    free(scriptInfo);
}
