//
//  CFASSFileDialogueTextContentOverrideContent.c
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "CFASSFileDialogueTextContentOverrideContent.h"
#include "CFUseTool.h"
#include "CFException.h"
#include "CFASSFileChange.h"
#include "CFASSFileChange_Private.h"
#include "CFASSFileDialogueTextDrawingContext.h"
#include "CFASSFileDialogueTextDrawingContext_Private.h"


struct CFASSFileDialogueTextContentOverrideContent {
    CFASSFileDialogueTextContentOverrideContentType type;
    union
    {
        struct
        {
            unsigned int weight;
        }blod;
        struct
        {
            bool isItalic;
        }italic;
        struct
        {
            bool isUnderline;
        }underline;
        struct
        {
            bool isStrikeOut;
        }strikeOut;
        struct
        {
            bool usingComplexed;
            CF2DAxis axis;
            double resolutionPixels;    // non-negative
        }border;
        struct
        {
            bool usingComplexed;
            CF2DAxis axis;
            double depth;           // non-negative
        }shadow;
        struct
        {
            bool usingGuassian;
            unsigned int strength;
        }blurEdge;
        struct
        {
            wchar_t *name;
        }fontName;
        struct
        {
            unsigned int resolutionPixels;
        }fontSize;
        struct
        {
            CF2DAxis axis;
            unsigned int percentage;
        }fontScale;
        struct
        {
            double resolutionPixels; // can be negative and have decimal point
        } spacing;
        struct
        {
            bool usingComplexed;
            CF3DAxis axis;
            int degrees;
        } rotation;
        struct
        {
            CF2DAxis axis;
            double factor;
        } shearing;
        struct
        {
            unsigned int identifier;
        }fontEncoding;
        struct
        {
            bool usingComplexed;
            unsigned int componentNumber;
            unsigned char blue, green, red;
        } color;
        struct
        {
            bool setAllComponent;
            unsigned int componentNumber;
            unsigned char transparent;
        } alpha;
        struct
        {
            bool legacy;
            unsigned int position;
        } alignment;
        struct
        {
            bool hasSweepEffect;
            bool removeBorderOutlineBeforeHighlight;
            unsigned int sentiSeconds;
        } karaokeDuration;
        struct
        {
            unsigned int style;
        } wrapStyle;
        struct
        {
            bool resetToDefault;
            wchar_t *styleName;     // NULL for reset to default
        } reset;
        struct
        {
            int x, y;
        } position;
        struct
        {
            int fromX, fromY;
            int toX, toY;
            bool hasTimeControl;
            unsigned int startFromMS, endFromMS;
        } movement;
        struct
        {
            int x, y;
        }rotationOrigin;
        struct
        {
            bool isComplexed;
            unsigned int fadeInMS, fadeOutMS;
            unsigned char beginAlpha, middleAlpha,endAlpha;      // 0-255, decimal
            unsigned int fadeInBeginMS, fadeInEndMS, fadeOutBeginMS, fadeOutEndMS;
        }fade;
        struct
        {
            bool hasTimeOffset;
            unsigned int beginOffsetMS, endOffsetMS;
            bool hasAcceleration;
            unsigned int acceleration;
            wchar_t *modifiers;
        }animation;
        struct
        {
            bool reverse;
            bool usingDrawingCommand;
            bool hasScale;
            int upLeftX, upLeftY;
            int lowRightX, lowRightY;
            unsigned int scale;
            wchar_t *drawingCommand;
        }clip;
        struct
        {
            unsigned int mode;
        }drawing;
        struct
        {
            int towardsBottomPixels;
        }baselineOffset;
    } data;
};

static bool CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(const wchar_t *contentString,
                                                                              const wchar_t *endPoint,
                                                                              const wchar_t *matchAlpha);

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(CFASSFileDialogueTextContentOverrideContentRef content);

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers(wchar_t *modifiers);

#pragma mark - Translation between strings

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCreateWithString(const wchar_t *contentString, const wchar_t *endPoint)
{
    if(contentString != NULL)
    {
        CFASSFileDialogueTextContentOverrideContentRef result;
        if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
        {
            int temp;
            if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* italic */
                                                                                 endPoint,
                                                                                 L"i"))
            {
                int isItalic;
                temp = swscanf(contentString, L"\\i%d", &isItalic);
                if(temp == 1 && isItalic>=0 && isItalic<=1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeItalic;
                    result->data.italic.isItalic = isItalic;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* blod */
                                                                                      endPoint,
                                                                                      L"b"))
            {
                int weight;
                temp = swscanf(contentString, L"\\b%d", &weight);
                if(temp == 1 && weight>=0)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeBlod;
                    result->data.blod.weight = weight;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* underline */
                                                                                      endPoint,
                                                                                      L"u"))
            {
                int isUnderline;
                temp = swscanf(contentString, L"\\u%d", &isUnderline);
                if(temp == 1 && isUnderline>=0 && isUnderline<=1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeUnderline;
                    result->data.underline.isUnderline = isUnderline;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* strikeOut */
                                                                                      endPoint,
                                                                                      L"s"))
            {
                int isStrikeOut;
                temp = swscanf(contentString, L"\\s%d", &isStrikeOut);
                if(temp == 1 && isStrikeOut>=0 && isStrikeOut<=1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeStrikeOut;
                    result->data.strikeOut.isStrikeOut = isStrikeOut;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* bord */
                                                                                      endPoint,
                                                                                      L"bord") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* bord x */
                                                                                      endPoint,
                                                                                      L"xbord") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* bord y */
                                                                                      endPoint,
                                                                                      L"ybord"))
            {
                double borderSize;
                if((temp = swscanf(contentString, L"\\bord%lf", &borderSize)) == 1)
                    result->data.border.usingComplexed = false;
                else if((temp = swscanf(contentString, L"\\xbord%lf", &borderSize)) == 1)
                    result->data.border.axis = CF2DAxisX;
                else if((temp = swscanf(contentString, L"\\ybord%lf", &borderSize)) == 1)
                    result->data.border.axis = CF2DAxisY;
                if(temp == 1 && borderSize>=0)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeBorder;
                    result->data.border.resolutionPixels = borderSize;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* shadow */
                                                                                      endPoint,
                                                                                      L"shad") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* shadow x */
                                                                                      endPoint,
                                                                                      L"xshad") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* shadow y */
                                                                                      endPoint,
                                                                                      L"yshad"))
            {
                double shadowDepth;
                if((temp = swscanf(contentString, L"\\shad%lf", &shadowDepth)) == 1)
                    result->data.shadow.usingComplexed = false;
                else if((temp = swscanf(contentString, L"\\xshad%lf", &shadowDepth)) == 1)
                    result->data.shadow.axis = CF2DAxisX;
                else if((temp = swscanf(contentString, L"\\yshad%lf", &shadowDepth)) == 1)
                    result->data.shadow.axis = CF2DAxisY;
                if(temp == 1 && shadowDepth>=0)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeShadow;
                    result->data.shadow.depth = shadowDepth;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* legacy blur */
                                                                                      endPoint,
                                                                                      L"be") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* guassian blur */
                                                                                      endPoint,
                                                                                      L"blur"))
            {
                unsigned int strength;
                if((temp = swscanf(contentString, L"\\be%u", &strength)) == 1)
                    result->data.blurEdge.usingGuassian = false;
                else if((temp = swscanf(contentString, L"\\blur%u", &strength)) == 1)
                    result->data.blurEdge.usingGuassian = true;
                if(temp == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeBlurEdge;
                    result->data.blurEdge.strength = strength;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* font size */
                                                                                      endPoint,
                                                                                      L"fs"))
            {
                unsigned int resolutionPixels;
                if((temp = swscanf(contentString, L"\\fs%u", &resolutionPixels)) == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSize;
                    result->data.fontSize.resolutionPixels = resolutionPixels;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* font scale x */
                                                                                      endPoint,
                                                                                      L"fscx") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* font scale y */
                                                                                      endPoint,
                                                                                      L"fscy"))
            {
                unsigned int percentage;
                if((temp = swscanf(contentString, L"\\fscx%u", &percentage)) == 1)
                    result->data.fontScale.axis = CF2DAxisX;
                else if((temp = swscanf(contentString, L"\\fscy%u", &percentage)) == 1)
                    result->data.fontScale.axis = CF2DAxisY;
                if(temp == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeFontScale;
                    result->data.fontScale.percentage = percentage;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* text spacing */
                                                                                      endPoint,
                                                                                      L"fsp"))
            {
                double resolutionPixels;
                if((temp = swscanf(contentString, L"\\fsp%lf", &resolutionPixels)) == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSpacing;
                    result->data.spacing.resolutionPixels = resolutionPixels;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* rotation default */
                                                                                      endPoint,
                                                                                      L"fr") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* rotation x */
                                                                                      endPoint,
                                                                                      L"frx") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* rotation y */
                                                                                      endPoint,
                                                                                      L"fry")  ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* rotation z */
                                                                                      endPoint,
                                                                                      L"frz"))
            {
                int degrees;
                if((temp = swscanf(contentString, L"\\fr%d", &degrees)) == 1)
                    result->data.rotation.usingComplexed = true;
                else if((temp = swscanf(contentString, L"\\frx%d", &degrees)) == 1)
                {
                    result->data.rotation.usingComplexed = false;
                    result->data.rotation.axis = CF3DAxisX;
                }
                else if((temp = swscanf(contentString, L"\\fry%d", &degrees)) == 1)
                {
                    result->data.rotation.usingComplexed = false;
                    result->data.rotation.axis = CF3DAxisY;
                }
                else if((temp = swscanf(contentString, L"\\frz%d", &degrees)) == 1)
                {
                    result->data.rotation.usingComplexed = false;
                    result->data.rotation.axis = CF3DAxisZ;
                }
                if(temp == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeTextRotation;
                    result->data.rotation.degrees = degrees;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* text shearing x */
                                                                                      endPoint,
                                                                                      L"fax") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* text shearing y */
                                                                                      endPoint,
                                                                                      L"fay"))
            {
                double factor;
                if((temp = swscanf(contentString, L"\\fax%lf", &factor)) == 1)
                    result->data.shearing.axis = CF2DAxisX;
                else if((temp = swscanf(contentString, L"\\fay%lf", &factor)) == 1)
                    result->data.shearing.axis = CF2DAxisY;
                if(temp == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeTextShearing;
                    result->data.shearing.factor = factor;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* font encoding */
                                                                                      endPoint,
                                                                                      L"fe"))
            {
                unsigned int identifier;
                if((temp = swscanf(contentString, L"\\fe%u", &identifier)) == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeTextEncoding;
                    result->data.fontEncoding.identifier = identifier;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* color default */
                                                                                      endPoint,
                                                                                      L"c") ||
                    CF_wchar_string_match_beginning(contentString, L"\\1c") ||
                    CF_wchar_string_match_beginning(contentString, L"\\2c") ||
                    CF_wchar_string_match_beginning(contentString, L"\\3c") ||
                    CF_wchar_string_match_beginning(contentString, L"\\4c"))
            {
                unsigned int componentNumber;
                unsigned long colorInfo;    // B G R
                bool checkMark = false;
                if((temp = swscanf(contentString, L"\\c&H%lx&", &colorInfo)) == 1)
                {
                    result->data.color.usingComplexed = false;
                    result->data.color.componentNumber = 0u;
                    checkMark = true;
                }
                else if((temp = swscanf(contentString, L"\\%uc&H%lx&", &componentNumber, &colorInfo)) == 2)
                    if(componentNumber>=1 && componentNumber<=4)
                    {
                        result->data.color.usingComplexed = true;
                        result->data.color.componentNumber = componentNumber;
                        checkMark = true;
                    }
                if(checkMark && colorInfo<=0xFFFFFF)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeColor;
                    result->data.color.red = colorInfo % (0xFF+1);
                    colorInfo /= (0xFF+1);
                    result->data.color.green = colorInfo % (0xFF+1);
                    colorInfo /= (0xFF+1);
                    result->data.color.blue = colorInfo % (0xFF+1);
                    return result;
                }
                
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* alpha all */
                                                                                      endPoint,
                                                                                      L"alpha") ||
                    CF_wchar_string_match_beginning(contentString, L"\\1a")  ||                           /* alpha 1 */
                    CF_wchar_string_match_beginning(contentString, L"\\2a")  ||                           /* alpha 1 */
                    CF_wchar_string_match_beginning(contentString, L"\\3a")  ||                           /* alpha 1 */
                    CF_wchar_string_match_beginning(contentString, L"\\4a"))                              /* alpha 1 */
            {
                unsigned int componentNumber;
                unsigned int transparent_uint;
                if((temp = swscanf(contentString, L"\\alpha&H%X&", &transparent_uint)) == 1)
                {
                    if(transparent_uint<= 0xFF)
                    {
                        result->type = CFASSFileDialogueTextContentOverrideContentTypeAlpha;
                        result->data.alpha.setAllComponent = true;
                        result->data.alpha.componentNumber = 0;
                        result->data.alpha.transparent = transparent_uint;
                        return result;
                    }
                }
                else if((temp = swscanf(contentString, L"\\%ua&H%X&", &componentNumber, &transparent_uint)) == 2)
                    if(transparent_uint<= 0xFF && componentNumber>=1 && componentNumber<=4)
                    {
                        result->type = CFASSFileDialogueTextContentOverrideContentTypeAlpha;
                        result->data.alpha.setAllComponent = false;
                        result->data.alpha.componentNumber = componentNumber;
                        result->data.alpha.transparent = transparent_uint;
                        return result;
                    }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* alignment legacy */
                                                                                      endPoint,
                                                                                      L"a") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* alignment numPad style */
                                                                                      endPoint,
                                                                                      L"an"))
            {
                unsigned int position;
                if((temp = swscanf(contentString, L"\\a%u", &position)) == 1)
                    result->data.alignment.legacy = true;
                else if((temp = swscanf(contentString, L"\\an%u", &position)) == 1)
                    result->data.alignment.legacy = false;
                if(temp == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeAlignment;
                    result->data.alignment.position = position;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* karaoke duration without sweep */
                                                                                      endPoint,
                                                                                      L"k") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* karaoke duration with sweep */
                                                                                      endPoint,
                                                                                      L"K") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* karaoke duration with sweep */
                                                                                      endPoint,
                                                                                      L"kf") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* karaoke duration remove b/s */
                                                                                      endPoint,
                                                                                      L"ko"))
            {
                unsigned int sentiSeconds;
                if((temp = swscanf(contentString, L"\\k%u", &sentiSeconds)) == 1)
                {
                    result->data.karaokeDuration.hasSweepEffect = false;
                    result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = false;
                }
                else if((temp = swscanf(contentString, L"\\K%u", &sentiSeconds)) == 1)
                {
                    result->data.karaokeDuration.hasSweepEffect = true;
                    result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = false;
                }
                else if((temp = swscanf(contentString, L"\\kf%u", &sentiSeconds)) == 1)
                {
                    result->data.karaokeDuration.hasSweepEffect = true;
                    result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = false;
                }
                else if((temp = swscanf(contentString, L"\\ko%u", &sentiSeconds)) == 1)
                {
                    result->data.karaokeDuration.hasSweepEffect = false;
                    result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = true;
                }
                if(temp == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration;
                    result->data.karaokeDuration.sentiSeconds = sentiSeconds;
                    return result;
                }
                
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* Wrap style */
                                                                                      endPoint,
                                                                                      L"q"))
            {
                unsigned int style;
                if((temp = swscanf(contentString, L"\\q%u", &style)) == 1)
                    if(style <= 3)
                    {
                        result->type = CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle;
                        result->data.wrapStyle.style = style;
                        return result;
                    }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* position */
                                                                                      endPoint,
                                                                                      L"pos"))
            {
                int x, y;
                if((temp = swscanf(contentString, L"\\pos(%d,%d)", &x, &y)) == 2)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypePosition;
                    result->data.position.x = x;
                    result->data.position.y = y;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* movement */
                                                                                      endPoint,
                                                                                      L"move"))
            {
                int fromX, fromY, toX, toY;
                unsigned int startFromMS, endFromMS;
                if((temp = swscanf(contentString, L"\\move(%d,%d,%d,%d,%u,%u)", &fromX, &fromY, &toX, &toY, &startFromMS, &endFromMS)) == 6)
                {
                    result->data.movement.hasTimeControl = true;
                    result->data.movement.startFromMS = startFromMS;
                    result->data.movement.endFromMS = endFromMS;
                }
                else if((temp = swscanf(contentString, L"\\move(%d,%d,%d,%d)", &fromX, &fromY, &toX, &toY)) == 4)
                {
                    result->data.movement.hasTimeControl = false;
                    result->data.movement.startFromMS = 0;
                    result->data.movement.endFromMS = 0;
                }
                if(temp >= 4)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeMove;
                    result->data.movement.fromX = fromX;
                    result->data.movement.fromY = fromY;
                    result->data.movement.toX = toX;
                    result->data.movement.toY = toY;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* rotation origin */
                                                                                      endPoint,
                                                                                      L"org"))
            {
                int x, y;
                if((temp = swscanf(contentString, L"\\org(%d,%d)", &x, &y)) == 2)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin;
                    result->data.rotationOrigin.x = x;
                    result->data.rotationOrigin.y = y;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* fade */
                                                                                      endPoint,
                                                                                      L"fad") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* fade complex */
                                                                                      endPoint,
                                                                                      L"fade"))
            {
                unsigned int fadeInMS, fadeOutMS,
                             beginAlpha, middleAlpha, endAlpha,
                             fadeInBeginMS, fadeInEndMS, fadeOutBeginMS, fadeOutEndMS;
                
                if((temp = swscanf(contentString, L"\\fad(%u,%u)", &fadeInMS, &fadeOutMS)) == 2)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeFade;
                    result->data.fade.isComplexed = false;
                    result->data.fade.fadeInMS = fadeInMS;
                    result->data.fade.fadeOutMS = fadeOutMS;
                    result->data.fade.beginAlpha = 0;
                    result->data.fade.middleAlpha = 0;
                    result->data.fade.endAlpha = 0;
                    result->data.fade.fadeInBeginMS = 0;
                    result->data.fade.fadeInEndMS = 0;
                    result->data.fade.fadeOutBeginMS = 0;
                    result->data.fade.fadeOutEndMS = 0;
                    return result;
                    
                }
                else if((temp = swscanf(contentString, L"\\fade(%u,%u,%u,%u,%u,%u,%u)",
                                        &beginAlpha, &middleAlpha, &endAlpha,
                                        &fadeInBeginMS, &fadeInEndMS,
                                        &fadeOutBeginMS, &fadeOutEndMS)) == 7)
                    if(beginAlpha<=0xFF && middleAlpha<=0xFF && endAlpha<=0xFF)
                    {
                        result->type = CFASSFileDialogueTextContentOverrideContentTypeFade;
                        result->data.fade.isComplexed = true;
                        result->data.fade.fadeInMS = fadeInMS;
                        result->data.fade.fadeOutMS = fadeOutMS;
                        result->data.fade.beginAlpha = beginAlpha;
                        result->data.fade.middleAlpha = middleAlpha;
                        result->data.fade.endAlpha = endAlpha;
                        result->data.fade.fadeInBeginMS = fadeInBeginMS;
                        result->data.fade.fadeInEndMS = fadeInEndMS;
                        result->data.fade.fadeOutBeginMS = fadeOutBeginMS;
                        result->data.fade.fadeOutEndMS = fadeOutEndMS;
                        return result;
                    }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* animation */
                                                                                      endPoint,
                                                                                      L"t"))
            {
                unsigned int beginOffsetMS, endOffsetMS;
                unsigned int acceleration;
                if((temp = swscanf(contentString, L"\\t(%u,%u,%u,\\", &beginOffsetMS, &endOffsetMS, &acceleration)) == 3)
                {
                    result->data.animation.hasAcceleration = true;
                    result->data.animation.hasTimeOffset = true;
                    result->data.animation.acceleration = acceleration;
                    result->data.animation.beginOffsetMS = beginOffsetMS;
                    result->data.animation.endOffsetMS = endOffsetMS;
                }
                else if((temp = swscanf(contentString, L"\\t(%u,%u,\\", &beginOffsetMS, &endOffsetMS)) == 2)
                {
                    result->data.animation.hasAcceleration = false;
                    result->data.animation.hasTimeOffset = true;
                    result->data.animation.beginOffsetMS = beginOffsetMS;
                    result->data.animation.endOffsetMS = endOffsetMS;
                }
                else if((temp = swscanf(contentString, L"\\t(%u,\\", &acceleration)) == 1)
                {
                    result->data.animation.hasAcceleration = true;
                    result->data.animation.hasTimeOffset = false;
                    result->data.animation.acceleration = acceleration;
                }
                if(temp >= 1)
                {
                    const wchar_t *dataBeginPoint = contentString + 1;
                    while(*dataBeginPoint != L'\\' && dataBeginPoint<endPoint) dataBeginPoint++;
                    bool checkPoint = true;
                    if(*dataBeginPoint == L'\\')
                    {
                        const wchar_t *dataEndPoint = dataBeginPoint;
                        while(*dataEndPoint!=L')' && dataEndPoint<endPoint && checkPoint)
                        {
                            if(*dataEndPoint == L'(')
                            {
                                while(*dataEndPoint != L')' && dataEndPoint<endPoint) dataEndPoint++;
                                if(*dataEndPoint != L')') checkPoint = false;
                            }
                            dataEndPoint++;
                        }
                        if(checkPoint && *dataEndPoint == L')')
                        {
                            dataEndPoint--;
                            if((result->data.animation.modifiers = malloc(sizeof(wchar_t)*(dataEndPoint-dataBeginPoint+1+1))) != NULL)
                            {
                                result->type = CFASSFileDialogueTextContentOverrideContentTypeAnimation;
                                wmemcpy(result->data.animation.modifiers, dataBeginPoint, dataEndPoint-dataBeginPoint+1);
                                result->data.animation.modifiers[dataEndPoint-dataBeginPoint+1] = L'\0';
                                if(CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers(result->data.animation.modifiers))
                                    return result;
                                free(result->data.animation.modifiers);
                            }
                        }
                    }
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* clip */
                                                                                      endPoint,
                                                                                      L"clip") ||
                    CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* clip reversed */
                                                                                      endPoint,
                                                                                      L"iclip"))
            {
                int upLeftX, upLeftY;
                int lowRightX, lowRightY;
                unsigned int scale;
                const wchar_t *dataBeginPoint = NULL, *dataEndPoint;
                bool checkMark = false;
                if((temp = swscanf(contentString, L"\\clip(%d,%d,%d,%d)", &upLeftX, &upLeftY, &lowRightX, &lowRightY)) == 4)
                {
                    result->data.clip.reverse = false;
                    result->data.clip.usingDrawingCommand = false;
                    result->data.clip.hasScale = false;
                    result->data.clip.upLeftX = upLeftX;
                    result->data.clip.upLeftY = upLeftY;
                    result->data.clip.lowRightX = lowRightX;
                    result->data.clip.lowRightY = lowRightY;
                    checkMark = true;
                }
                else if((temp = swscanf(contentString, L"\\iclip(%d,%d,%d,%d)", &upLeftX, &upLeftY, &lowRightX, &lowRightY)) == 4)
                {
                    result->data.clip.reverse = true;
                    result->data.clip.usingDrawingCommand = false;
                    result->data.clip.hasScale = false;
                    result->data.clip.upLeftX = upLeftX;
                    result->data.clip.upLeftY = upLeftY;
                    result->data.clip.lowRightX = lowRightX;
                    result->data.clip.lowRightY = lowRightY;
                    checkMark = true;
                }
                else if((temp = swscanf(contentString, L"\\clip(%u,%*l[^)])", &scale)) == 1)
                {
                    dataBeginPoint = contentString;
                    while (*dataBeginPoint!=L'(') dataBeginPoint++;
                    dataBeginPoint++;
                    result->data.clip.reverse = false;
                    result->data.clip.usingDrawingCommand = true;
                    result->data.clip.hasScale = true;
                    result->data.clip.scale = scale;
                    checkMark = true;
                }
                else if((temp = swscanf(contentString, L"\\iclip(%u,%*l[^)])", &scale)) == 1)
                {
                    dataBeginPoint = contentString;
                    while (*dataBeginPoint!=L'(') dataBeginPoint++;
                    dataBeginPoint++;
                    result->data.clip.reverse = true;
                    result->data.clip.usingDrawingCommand = true;
                    result->data.clip.hasScale = true;
                    result->data.clip.scale = scale;
                    checkMark = true;
                }
                else if(CF_wchar_string_match_beginning(contentString, L"\\clip("))
                {
                    dataBeginPoint = contentString + wcslen(L"\\clip(");
                    result->data.clip.reverse = false;
                    result->data.clip.usingDrawingCommand = true;
                    result->data.clip.hasScale = false;
                    checkMark = true;
                }
                else if(CF_wchar_string_match_beginning(contentString, L"\\iclip("))
                {
                    dataBeginPoint = contentString + wcslen(L"\\iclip(");
                    result->data.clip.reverse = true;
                    result->data.clip.usingDrawingCommand = true;
                    result->data.clip.hasScale = false;
                    checkMark = true;
                }
                if(checkMark)       // manage clip.drawingCommand
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeClip;
                    if(result->data.clip.usingDrawingCommand)
                    {
                        if(result->data.clip.hasScale)
                        {
                            while (*dataBeginPoint!=L',')
                                dataBeginPoint++;
                            if(dataBeginPoint<endPoint) dataBeginPoint++;
                        }
                        dataEndPoint = dataBeginPoint;
                        while(*dataEndPoint!=L')' && dataEndPoint<endPoint) dataEndPoint++;
                        if(*dataEndPoint == L')')
                        {
                            dataEndPoint--;
                            if(dataEndPoint>=dataBeginPoint)
                                if((result->data.clip.drawingCommand = malloc(sizeof(wchar_t)*(dataEndPoint-dataBeginPoint+1+1)))!=NULL)
                                {
                                    wmemcpy(result->data.clip.drawingCommand, dataBeginPoint, dataEndPoint-dataBeginPoint+1);
                                    result->data.clip.drawingCommand[dataEndPoint-dataBeginPoint+1] = L'\0';
                                    CFASSFileDialogueTextDrawingContextRef drawingContext;
                                    if((drawingContext = CFASSFileDialogueTextDrawingContextCreateFromString(result->data.clip.drawingCommand)) != NULL)
                                    {
                                        CFASSFileDialogueTextDrawingContextDestory(drawingContext);
                                        return result;
                                    }
                                    free(result->data.clip.drawingCommand);
                                }
                        }
                    }
                    else
                    {
                        result->data.clip.drawingCommand = NULL;
                        return result;
                    }
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* drawing mode */
                                                                                      endPoint,
                                                                                      L"p"))
            {
                unsigned int mode;
                if((temp = swscanf(contentString, L"\\p%u", &mode)) == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeDrawing;
                    result->data.drawing.mode = mode;
                    return result;
                }
            }
            else if(CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(contentString,     /* baseline offset */
                                                                                      endPoint,
                                                                                      L"pbo"))
            {
                int towardsBottomPixels;
                if((temp = swscanf(contentString, L"\\pbo%d", &towardsBottomPixels)) == 1)
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset;
                    result->data.baselineOffset.towardsBottomPixels = towardsBottomPixels;
                    return result;
                }
            }
            else if(CF_wchar_string_match_beginning(contentString, L"\\fn"))                             /* font name */
            {
                const wchar_t *dataBeginPoint = contentString + wcslen(L"\\fn"), *dataEndPoint;
                if(dataBeginPoint <= endPoint && *dataBeginPoint!=L'\\')
                {
                    dataEndPoint = dataBeginPoint;
                    while(dataEndPoint<endPoint) dataEndPoint++;
                    if((result->data.fontName.name = malloc(sizeof(wchar_t)*(dataEndPoint-dataBeginPoint+1+1))) != NULL)
                    {
                        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontName;
                        wmemcpy(result->data.fontName.name, dataBeginPoint, dataEndPoint-dataBeginPoint+1);
                        result->data.fontName.name[dataEndPoint-dataBeginPoint+1] = L'\0';
                        return result;
                    }
                }
            }
            else if(CF_wchar_string_match_beginning(contentString, L"\\r"))                              /* reset */
            {
                const wchar_t *dataBeginPoint = contentString + wcslen(L"\\r"), *dataEndPoint;
                if(dataBeginPoint <= endPoint)
                {
                    dataEndPoint = dataBeginPoint;
                    while(dataEndPoint<endPoint) dataEndPoint++;
                    if((result->data.reset.styleName = malloc(sizeof(wchar_t)*(dataEndPoint-dataBeginPoint+1+1))) != NULL)
                    {
                        result->type = CFASSFileDialogueTextContentOverrideContentTypeReset;
                        result->data.reset.resetToDefault = false;
                        wmemcpy(result->data.reset.styleName, dataBeginPoint, dataEndPoint-dataBeginPoint+1);
                        result->data.reset.styleName[dataEndPoint-dataBeginPoint+1] = L'\0';
                        return result;
                    }
                }
                else
                {
                    result->type = CFASSFileDialogueTextContentOverrideContentTypeReset;
                    result->data.reset.resetToDefault = true;
                    result->data.reset.styleName = NULL;
                    return result;
                }
            }
            free(result);
        }
    }
    return NULL;
}

static bool CFASSFileDialogueTextContentOverrideContentCompareStringAndPrefix(const wchar_t *contentString,
                                                                              const wchar_t *endPoint,
                                                                              const wchar_t *matchAlpha)
{
    if(*contentString != L'\\') return false;
    contentString++;
    const wchar_t *searchEnd = contentString - 1;
    while(iswalpha(searchEnd[1])) searchEnd++;
    if(searchEnd<contentString) return false;
    while (contentString <= searchEnd)
        if(*matchAlpha++ != *contentString++) return false;
    if(*matchAlpha != L'\0') return false;
    return true;
}

int CFASSFileDialogueTextContentOverrideContentStoreStringResult(CFASSFileDialogueTextContentOverrideContentRef overrideContent, wchar_t *targetPoint)
{
    int result = -1;
    unsigned long colorInfo;
    if(targetPoint == NULL)
    {
        FILE *fp = tmpfile();
        if(fp == NULL) return -1;
        
        switch (overrideContent->type) {
            case CFASSFileDialogueTextContentOverrideContentTypeBlod:
                result = fwprintf(fp, L"\\b%u", overrideContent->data.blod.weight);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeItalic:
                result = fwprintf(fp, L"\\i%lc", overrideContent->data.italic.isItalic?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeUnderline:
                result = fwprintf(fp, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeStrikeOut:
                result = fwprintf(fp, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBorder:
                if(overrideContent->data.border.usingComplexed)
                    result = fwprintf(fp, L"\\%lcbord%g",
                                      overrideContent->data.border.axis == CF2DAxisX?L'x':L'y',
                                      overrideContent->data.border.resolutionPixels);
                else
                    result = fwprintf(fp, L"\\bord%g", overrideContent->data.border.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeShadow:
                if(overrideContent->data.shadow.usingComplexed)
                    result = fwprintf(fp, L"\\%lcshad%g",
                                      overrideContent->data.shadow.axis == CF2DAxisX ? L'x':L'y',
                                      overrideContent->data.shadow.depth);
                else
                    result = fwprintf(fp, L"\\shad%g", overrideContent->data.shadow.depth);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBlurEdge:
                if(overrideContent->data.blurEdge.usingGuassian)
                    result = fwprintf(fp, L"\\blur%u", overrideContent->data.blurEdge.strength);
                else
                    result = fwprintf(fp, L"\\be%u", overrideContent->data.blurEdge.strength);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontName:
                result = fwprintf(fp, L"\\fn%ls", overrideContent->data.fontName.name);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSize:
                result = fwprintf(fp, L"\\fs%u", overrideContent->data.fontSize.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontScale:
                result = fwprintf(fp, L"\\fsc%lc%u",
                                  overrideContent->data.fontScale.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.fontScale.percentage);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSpacing:
                result = fwprintf(fp, L"\\fsp%g", overrideContent->data.spacing.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextRotation:
                if(overrideContent->data.rotation.usingComplexed)
                    result = fwprintf(fp, L"\\fr%lc%d",
                                      overrideContent->data.rotation.axis == CF3DAxisX?L'x':
                                      overrideContent->data.rotation.axis == CF3DAxisY?L'y':L'z',
                                      overrideContent->data.rotation.degrees);
                else
                    result = fwprintf(fp, L"\\fr%d", overrideContent->data.rotation.degrees);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextShearing:
                result = fwprintf(fp, L"\\fa%lc%g",
                                  overrideContent->data.shearing.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.shearing.factor);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextEncoding:
                result = fwprintf(fp, L"\\fe%u", overrideContent->data.fontEncoding.identifier);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeColor:
                colorInfo =
                      overrideContent->data.color.red
                    + overrideContent->data.color.green*(0xFF+1)
                    + overrideContent->data.color.blue *(0xFFFF+1);
                if(overrideContent->data.color.usingComplexed)
                    result = fwprintf(fp, L"\\%uc&H%lX&",
                                      overrideContent->data.color.componentNumber,
                                      colorInfo);
                else
                    result = fwprintf(fp, L"\\c&H%lX&", colorInfo);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlpha:
                if(overrideContent->data.alpha.setAllComponent)
                    result = fwprintf(fp, L"\\alpha&H%X&", (unsigned int)overrideContent->data.alpha.transparent);
                else
                    result = fwprintf(fp, L"\\%ua&H%X&",
                                      overrideContent->data.alpha.componentNumber,
                                      (unsigned int)overrideContent->data.alpha.transparent);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlignment:
                if(overrideContent->data.alignment.legacy)
                    result = fwprintf(fp, L"\\a%u", overrideContent->data.alignment.position);
                else
                    result = fwprintf(fp, L"\\an%u", overrideContent->data.alignment.position);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration:
                if(overrideContent->data.karaokeDuration.hasSweepEffect)
                    result = fwprintf(fp, L"\\kf%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else if(overrideContent->data.karaokeDuration.removeBorderOutlineBeforeHighlight)
                    result = fwprintf(fp, L"\\ko%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else
                    result = fwprintf(fp, L"\\k%u", overrideContent->data.karaokeDuration.sentiSeconds);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle:
                result = fwprintf(fp, L"\\q%u", overrideContent->data.wrapStyle.style);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeReset:
                if(overrideContent->data.reset.resetToDefault)
                    result = fwprintf(fp, L"\\r");
                else
                    result = fwprintf(fp, L"\\r%ls", overrideContent->data.reset.styleName);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAnimation:
                if(overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = fwprintf(fp, L"\\t(%u,%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else if(overrideContent->data.animation.hasTimeOffset && !overrideContent->data.animation.hasAcceleration)
                    result = fwprintf(fp, L"\\t(%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.modifiers);
                else if(!overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = fwprintf(fp, L"\\t(%u,%ls)",
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else
                    result = fwprintf(fp, L"\\t(%ls)", overrideContent->data.animation.modifiers);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeMove:
                if(overrideContent->data.movement.hasTimeControl)
                    result = fwprintf(fp, L"\\move(%d,%d,%d,%d,%u,%u)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY,
                                      overrideContent->data.movement.startFromMS,
                                      overrideContent->data.movement.endFromMS);
                else
                    result = fwprintf(fp, L"\\move(%d,%d,%d,%d)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypePosition:
                result = fwprintf(fp, L"\\pos(%d,%d)", overrideContent->data.position.x, overrideContent->data.position.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin:
                result = fwprintf(fp, L"\\org(%d,%d)",
                                  overrideContent->data.rotationOrigin.x,
                                  overrideContent->data.rotationOrigin.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFade:
                if(overrideContent->data.fade.isComplexed)
                    result = fwprintf(fp, L"\\fade(%u,%u,%u,%u,%u,%u,%u)",
                                      (unsigned int)overrideContent->data.fade.beginAlpha,
                                      (unsigned int)overrideContent->data.fade.middleAlpha,
                                      (unsigned int)overrideContent->data.fade.endAlpha,
                                      overrideContent->data.fade.fadeInBeginMS,
                                      overrideContent->data.fade.fadeInEndMS,
                                      overrideContent->data.fade.fadeOutBeginMS,
                                      overrideContent->data.fade.fadeOutEndMS);
                else
                    result = fwprintf(fp, L"\\fad(%u,%u)",
                                      overrideContent->data.fade.fadeInMS,
                                      overrideContent->data.fade.fadeOutMS);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeClip:
                if(overrideContent->data.clip.usingDrawingCommand)
                    if(overrideContent->data.clip.hasScale)
                        result = fwprintf(fp, L"\\%lsclip(%u,%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.scale,
                                          overrideContent->data.clip.drawingCommand);
                    else
                        result = fwprintf(fp, L"\\%lsclip(%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.drawingCommand);
                else
                    result = fwprintf(fp, L"\\%lsclip(%d,%d,%d,%d)",
                                      overrideContent->data.clip.reverse?L"i":L"",
                                      overrideContent->data.clip.upLeftX,
                                      overrideContent->data.clip.upLeftY,
                                      overrideContent->data.clip.lowRightX,
                                      overrideContent->data.clip.lowRightY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeDrawing:
                result = fwprintf(fp, L"\\p%u", overrideContent->data.drawing.mode);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset:
                result = fwprintf(fp, L"\\pbo%d", overrideContent->data.baselineOffset.towardsBottomPixels);
                break;
        }
        fclose(fp);
    }
    else
        switch (overrideContent->type) {
            case CFASSFileDialogueTextContentOverrideContentTypeBlod:
                result = swprintf(targetPoint,SIZE_MAX, L"\\b%u", overrideContent->data.blod.weight);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeItalic:
                result = swprintf(targetPoint,SIZE_MAX, L"\\i%lc", overrideContent->data.italic.isItalic?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeUnderline:
                result = swprintf(targetPoint,SIZE_MAX, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeStrikeOut:
                result = swprintf(targetPoint,SIZE_MAX, L"\\u%lc", overrideContent->data.underline.isUnderline?L'1':L'0');
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBorder:
                if(overrideContent->data.border.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%lcbord%g",
                                      overrideContent->data.border.axis == CF2DAxisX?L'x':L'y',
                                      overrideContent->data.border.resolutionPixels);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\bord%g", overrideContent->data.border.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeShadow:
                if(overrideContent->data.shadow.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%lcshad%g",
                                      overrideContent->data.shadow.axis == CF2DAxisX ? L'x':L'y',
                                      overrideContent->data.shadow.depth);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\shad%g", overrideContent->data.shadow.depth);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBlurEdge:
                if(overrideContent->data.blurEdge.usingGuassian)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\blur%u", overrideContent->data.blurEdge.strength);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\be%u", overrideContent->data.blurEdge.strength);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontName:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fn%ls", overrideContent->data.fontName.name);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSize:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fs%u", overrideContent->data.fontSize.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontScale:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fsc%lc%u",
                                  overrideContent->data.fontScale.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.fontScale.percentage);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFontSpacing:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fsp%g", overrideContent->data.spacing.resolutionPixels);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextRotation:
                if(overrideContent->data.rotation.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fr%lc%d",
                                      overrideContent->data.rotation.axis == CF3DAxisX?L'x':
                                      overrideContent->data.rotation.axis == CF3DAxisY?L'y':L'z',
                                      overrideContent->data.rotation.degrees);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fr%d", overrideContent->data.rotation.degrees);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextShearing:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fa%lc%g",
                                  overrideContent->data.shearing.axis == CF2DAxisX?L'x':L'y',
                                  overrideContent->data.shearing.factor);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeTextEncoding:
                result = swprintf(targetPoint,SIZE_MAX, L"\\fe%u", overrideContent->data.fontEncoding.identifier);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeColor:
                colorInfo =
                overrideContent->data.color.red
                + overrideContent->data.color.green*(0xFF+1)
                + overrideContent->data.color.blue *(0xFFFF+1);
                if(overrideContent->data.color.usingComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%uc&H%lX&",
                                      overrideContent->data.color.componentNumber,
                                      colorInfo);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\c&H%lX&", colorInfo);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlpha:
                if(overrideContent->data.alpha.setAllComponent)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\alpha&H%X&", (unsigned int)overrideContent->data.alpha.transparent);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\%ua&H%X&",
                                      overrideContent->data.alpha.componentNumber,
                                      (unsigned int)overrideContent->data.alpha.transparent);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAlignment:
                if(overrideContent->data.alignment.legacy)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\a%u", overrideContent->data.alignment.position);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\an%u", overrideContent->data.alignment.position);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration:
                if(overrideContent->data.karaokeDuration.hasSweepEffect)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\kf%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else if(overrideContent->data.karaokeDuration.removeBorderOutlineBeforeHighlight)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\ko%u", overrideContent->data.karaokeDuration.sentiSeconds);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\k%u", overrideContent->data.karaokeDuration.sentiSeconds);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle:
                result = swprintf(targetPoint,SIZE_MAX, L"\\q%u", overrideContent->data.wrapStyle.style);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeReset:
                if(overrideContent->data.reset.resetToDefault)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\r");
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\r%ls", overrideContent->data.reset.styleName);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAnimation:
                if(overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%u,%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else if(overrideContent->data.animation.hasTimeOffset && !overrideContent->data.animation.hasAcceleration)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%u,%u,%ls)",
                                      overrideContent->data.animation.beginOffsetMS,
                                      overrideContent->data.animation.endOffsetMS,
                                      overrideContent->data.animation.modifiers);
                else if(!overrideContent->data.animation.hasTimeOffset && overrideContent->data.animation.hasAcceleration)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%u,%ls)",
                                      overrideContent->data.animation.acceleration,
                                      overrideContent->data.animation.modifiers);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\t(%ls)", overrideContent->data.animation.modifiers);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeMove:
                if(overrideContent->data.movement.hasTimeControl)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\move(%d,%d,%d,%d,%u,%u)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY,
                                      overrideContent->data.movement.startFromMS,
                                      overrideContent->data.movement.endFromMS);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\move(%d,%d,%d,%d)",
                                      overrideContent->data.movement.fromX,
                                      overrideContent->data.movement.fromY,
                                      overrideContent->data.movement.toX,
                                      overrideContent->data.movement.toY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypePosition:
                result = swprintf(targetPoint,SIZE_MAX, L"\\pos(%d,%d)", overrideContent->data.position.x, overrideContent->data.position.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin:
                result = swprintf(targetPoint,SIZE_MAX, L"\\org(%d,%d)",
                                  overrideContent->data.rotationOrigin.x,
                                  overrideContent->data.rotationOrigin.y);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeFade:
                if(overrideContent->data.fade.isComplexed)
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fade(%u,%u,%u,%u,%u,%u,%u)",
                                      (unsigned int)overrideContent->data.fade.beginAlpha,
                                      (unsigned int)overrideContent->data.fade.middleAlpha,
                                      (unsigned int)overrideContent->data.fade.endAlpha,
                                      overrideContent->data.fade.fadeInBeginMS,
                                      overrideContent->data.fade.fadeInEndMS,
                                      overrideContent->data.fade.fadeOutBeginMS,
                                      overrideContent->data.fade.fadeOutEndMS);
                else
                    result = swprintf(targetPoint,SIZE_MAX, L"\\fad(%u,%u)",
                                      overrideContent->data.fade.fadeInMS,
                                      overrideContent->data.fade.fadeOutMS);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeClip:
                if(overrideContent->data.clip.usingDrawingCommand)
                    if(overrideContent->data.clip.hasScale)
                        result = swprintf(targetPoint,SIZE_MAX, L"\\%lsclip(%u,%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.scale,
                                          overrideContent->data.clip.drawingCommand);
                    else
                        result = swprintf(targetPoint,SIZE_MAX, L"\\%lsclip(%ls)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.drawingCommand);
                    else
                        result = swprintf(targetPoint,SIZE_MAX, L"\\%lsclip(%d,%d,%d,%d)",
                                          overrideContent->data.clip.reverse?L"i":L"",
                                          overrideContent->data.clip.upLeftX,
                                          overrideContent->data.clip.upLeftY,
                                          overrideContent->data.clip.lowRightX,
                                          overrideContent->data.clip.lowRightY);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeDrawing:
                result = swprintf(targetPoint,SIZE_MAX, L"\\p%u", overrideContent->data.drawing.mode);
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset:
                result = swprintf(targetPoint,SIZE_MAX, L"\\pbo%d", overrideContent->data.baselineOffset.towardsBottomPixels);
                break;
        }
    if(result < 0) return -1;
    return result;
}

void CFASSFileDialogueTextContentOverrideContentDestory(CFASSFileDialogueTextContentOverrideContentRef overrideContent)
{
    if(overrideContent == NULL) return;
    // Clip.drawingCommand  may exist
    // Animation.modifiers
    // Reset.styleName      may exist
    // FontName.name
    if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeClip)
    {
        if(overrideContent->data.clip.usingDrawingCommand)
            free(overrideContent->data.clip.drawingCommand);
    }
    else if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeAnimation)
        free(overrideContent->data.animation.modifiers);
    else if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeReset)
    {
        if(!overrideContent->data.reset.resetToDefault)
            free(overrideContent->data.reset.styleName);
    }
    else if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeFontName)
        free(overrideContent->data.fontName.name);
    free(overrideContent);
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCopy(CFASSFileDialogueTextContentOverrideContentRef overrideContent)
{
    // fontName reset animation clip
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        *result = *overrideContent;
        bool checkMark = true;
        switch (overrideContent->type) {
            case CFASSFileDialogueTextContentOverrideContentTypeFontName:
                if((result->data.fontName.name = CF_Dump_wchar_string(overrideContent->data.fontName.name)) == NULL)
                    checkMark = false;
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeReset:
                if(!overrideContent->data.reset.resetToDefault)
                    if((result->data.reset.styleName = CF_Dump_wchar_string(overrideContent->data.reset.styleName)) == NULL)
                        checkMark = false;
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeAnimation:
                if((result->data.animation.modifiers = CF_Dump_wchar_string(overrideContent->data.animation.modifiers)) == NULL)
                    checkMark = false;
                break;
            case CFASSFileDialogueTextContentOverrideContentTypeClip:
                if(overrideContent->data.clip.usingDrawingCommand)
                    if((result->data.clip.drawingCommand = CF_Dump_wchar_string(overrideContent->data.clip.drawingCommand)) == NULL)
                        checkMark = false;
                break;
            default:
                // do nothing
                break;
        }
        if(checkMark)
            return result;
        free(result);
    }
    return NULL;
}

#pragma mark - Specific Override Context create

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBlodContentCreate(unsigned int blod)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBlod;
        result->data.blod.weight = blod;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideItalicContentCreate(bool isItalic)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeItalic;
        result->data.italic.isItalic = isItalic;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideUnderlineContentCreate(bool isUnderline)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeUnderline;
        result->data.underline.isUnderline = isUnderline;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideStrikeOutContentCreate(bool isStrikeOut)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeStrikeOut;
        result->data.strikeOut.isStrikeOut = isStrikeOut;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBorderSizeContentCreate(bool usingComplexed,
                                                                                                           CF2DAxis axis,
                                                                                                           double resolutionPixels)
{
    if(resolutionPixels<0) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBorder;
        result->data.border.usingComplexed = usingComplexed;
        result->data.border.axis = axis;
        result->data.border.resolutionPixels = resolutionPixels;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideShadowContentCreate(bool usingComplexed,
                                                                                                       CF2DAxis axis,
                                                                                                       double depth)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeShadow;
        result->data.shadow.usingComplexed = usingComplexed;
        result->data.shadow.axis = axis;
        result->data.shadow.depth = depth;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBlurEdgeContentCreate(bool usingGuassian,
                                                                                                         unsigned int strength)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBlurEdge;
        result->data.blurEdge.usingGuassian = usingGuassian;
        result->data.blurEdge.strength = strength;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontNameContentCreate(wchar_t *fontName)
{
    if(fontName == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        if((result->data.fontName.name = CF_Dump_wchar_string(fontName)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentOverrideContentTypeFontName;
            return result;
        }
        free(result);
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontSizeContentCreate(unsigned int resolutionPixels)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSize;
        result->data.fontSize.resolutionPixels = resolutionPixels;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontScaleContentCreate(CF2DAxis axis,
                                                                                                          unsigned int percentage)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontScale;
        result->data.fontScale.axis = axis;
        result->data.fontScale.percentage = percentage;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontSpacingContentCreate(double resolutionPixels)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFontSpacing;
        result->data.spacing.resolutionPixels = resolutionPixels;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextRotationContentCreate(bool defineAxis,
                                                                                                             CF3DAxis axis,
                                                                                                             int degrees)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeTextRotation;
        result->data.rotation.usingComplexed = defineAxis;
        result->data.rotation.axis = axis;
        result->data.rotation.degrees = degrees;
        
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextShearingContentCreate(CF2DAxis axis,
                                                                                                             double factor)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeTextShearing;
        result->data.shearing.axis = axis;
        result->data.shearing.factor = factor;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextEncodingContentCreate(unsigned int textEncoding)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeTextEncoding;
        result->data.fontEncoding.identifier = textEncoding;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideColorContentCreate(bool definesWhichColor,
                                                                                                      unsigned int colorCount,
                                                                                                      unsigned char blue,
                                                                                                      unsigned char green,
                                                                                                      unsigned char red)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeColor;
        result->data.color.usingComplexed = definesWhichColor;
        result->data.color.componentNumber = colorCount;
        result->data.color.blue = blue;
        result->data.color.green = green;
        result->data.color.red = red;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAlphaContentCreate(bool setAllComponent,
                                                                                                      unsigned int alphaCount,
                                                                                                      unsigned char alpha)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeAlpha;
        result->data.alpha.setAllComponent = setAllComponent;
        result->data.alpha.componentNumber = alphaCount;
        result->data.alpha.transparent = alpha;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAlignmentContentCreate(bool isLegacyStyle, unsigned int alignmentPosition)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeAlignment;
        result->data.alignment.legacy = isLegacyStyle;
        result->data.alignment.position = alignmentPosition;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideKaraokeDurationContentCreate(bool isSweep, unsigned int centiSeconds, bool removeBorderAndShadow)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration;
        result->data.karaokeDuration.hasSweepEffect = isSweep;
        result->data.karaokeDuration.sentiSeconds = centiSeconds;
        result->data.karaokeDuration.removeBorderOutlineBeforeHighlight = removeBorderAndShadow;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideWrapStyleContentCreate(unsigned int wrapStyle)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle;
        result->data.wrapStyle.style = wrapStyle;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideResetContentCreate(bool resetToDefault, wchar_t *styleName)
{
    if(!resetToDefault && styleName == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        if((result->data.reset.styleName = CF_Dump_wchar_string(styleName)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentOverrideContentTypeReset;
            result->data.reset.resetToDefault = resetToDefault;
            if(!resetToDefault)
            {
                if((result->data.reset.styleName = CF_Dump_wchar_string(styleName)) != NULL)
                    return result;
            }
            else
                return result;
        }
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverridePositionContentCreate(int x, int y)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypePosition;
        result->data.position.x = x;
        result->data.position.y = y;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideMovementContentCreate(int fromX,
                                                                                                         int fromY,
                                                                                                         int toX,
                                                                                                         int toY,
                                                                                                         bool hasTimeControl,
                                                                                                         unsigned int startFromMS,
                                                                                                         unsigned int endFromMS)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeMove;
        result->data.movement.fromX = fromX;
        result->data.movement.fromY = fromY;
        result->data.movement.toX = toX;
        result->data.movement.toY = toY;
        result->data.movement.hasTimeControl = hasTimeControl;
        result->data.movement.startFromMS = startFromMS;
        result->data.movement.endFromMS = endFromMS;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideRotationOriginContentCreate(int x, int y)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin;
        result->data.rotationOrigin.x = x;
        result->data.rotationOrigin.y = y;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFadeContentCreate(bool isComplexed,
                                                                                                     unsigned int fadeInMS,
                                                                                                     unsigned int fadeOutMS,
                                                                                                     unsigned char beginAlpha,
                                                                                                     unsigned char middleAlpha,
                                                                                                     unsigned char endAlpha,
                                                                                                     unsigned int fadeInBeginMS,
                                                                                                     unsigned int fadeInEndMS,
                                                                                                     unsigned int fadeOutBeginMS,
                                                                                                     unsigned int fadeOutEndMS)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeFade;
        result->data.fade.isComplexed = isComplexed;
        result->data.fade.fadeInMS = fadeInMS;
        result->data.fade.fadeOutMS = fadeOutMS;
        result->data.fade.beginAlpha = beginAlpha;
        result->data.fade.middleAlpha = middleAlpha;
        result->data.fade.endAlpha = endAlpha;
        result->data.fade.fadeInBeginMS = fadeInBeginMS;
        result->data.fade.fadeInEndMS = fadeInEndMS;
        result->data.fade.fadeOutBeginMS = fadeOutBeginMS;
        result->data.fade.fadeOutEndMS = fadeOutEndMS;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAnimationContentCreate(bool hasTimeOffset,
                                                                                                          unsigned int beginOffsetMS,
                                                                                                          unsigned int endOffsetMS,
                                                                                                          bool hasAcceleration,
                                                                                                          unsigned int acceleration,
                                                                                                          const wchar_t *modifiers)
{
    if(modifiers == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        if((result->data.animation.modifiers = CF_Dump_wchar_string(modifiers)) != NULL)
        {
            result->type = CFASSFileDialogueTextContentOverrideContentTypeAnimation;
            result->data.animation.hasTimeOffset = hasTimeOffset;
            result->data.animation.beginOffsetMS = beginOffsetMS;
            result->data.animation.endOffsetMS = endOffsetMS;
            result->data.animation.hasAcceleration = hasAcceleration;
            result->data.animation.acceleration = acceleration;
            return result;
        }
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideClipContentCreate(bool reverse,
                                                                                                     bool usingDrawingCommand,
                                                                                                     bool hasScale,
                                                                                                     int upLeftX, int upLeftY,
                                                                                                     int lowRightX, int lowRightY,
                                                                                                     unsigned int scale,
                                                                                                     wchar_t *drawingCommand)
{
    if(usingDrawingCommand && drawingCommand == NULL) return NULL;
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeClip;
        result->data.clip.usingDrawingCommand = usingDrawingCommand;
        result->data.clip.hasScale = hasScale;
        result->data.clip.upLeftX = upLeftX;
        result->data.clip.upLeftY = upLeftY;
        result->data.clip.lowRightX = lowRightX;
        result->data.clip.lowRightY = lowRightY;
        result->data.clip.scale = scale;
        if(usingDrawingCommand)
        {
            if((result->data.clip.drawingCommand = CF_Dump_wchar_string(drawingCommand)) != NULL)
                return result;
        }
        else
        {
            result->data.clip.drawingCommand = NULL;
            return result;
        }
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideDrawingContentCreate(unsigned int drawingMode)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeDrawing;
        result->data.drawing.mode = drawingMode;
        return result;
    }
    return NULL;
}

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBaselineOffsetContentCreate(int towardsBottomPixels)
{
    CFASSFileDialogueTextContentOverrideContentRef result;
    if((result = malloc(sizeof(struct CFASSFileDialogueTextContentOverrideContent))) != NULL)
    {
        result->type = CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset;
        result->data.baselineOffset.towardsBottomPixels = towardsBottomPixels;
        return result;
    }
    return NULL;
}

#pragma mark - Other function

void CFASSFileDialogueTextContentOverrideContentMakeChange(CFASSFileDialogueTextContentOverrideContentRef overrideContent, CFASSFileChangeRef change)
{
    if(overrideContent == NULL || change == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverrideContent %p MakeChange %p", overrideContent, change);
    if(change->type & CFASSFileChangeTypeFontSize)
    {
        if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeFontSize && change->fontSize.affectOverride)
        {
            if(change->fontSize.byPercentage)
                overrideContent->data.fontSize.resolutionPixels *= change->fontSize.percentage;
            else
                overrideContent->data.fontSize.resolutionPixels = change->fontSize.fontSize;
        }
    }
    if(change->type & CFASSFileChangeTypeFontName)
    {
        if(overrideContent->type == CFASSFileDialogueTextContentOverrideContentTypeFontName && change->fontSize.affectOverride)
        {
            wchar_t *dumped = CF_Dump_wchar_string(change->fontName.fontName);
            if(dumped != NULL)
            {
                free(overrideContent->data.fontName.name);
                overrideContent->data.fontName.name = dumped;
            }
        }
    }
}

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers(wchar_t *modifiers)
{
    if(modifiers == NULL)
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CFASSFileDialogueTextContentOverrideContentCheckAnimationModifiers NULL");
    if(*modifiers!=L'\\')   // include modifier[0] == L'\0'
        return false;
    wchar_t *nextBeginPoint;
    CFASSFileDialogueTextContentOverrideContentRef modifierContent;
    while ((nextBeginPoint = wcschr(modifiers+1, L'\\'))!=NULL)
    {
        modifierContent = CFASSFileDialogueTextContentOverrideContentCreateWithString(modifiers, nextBeginPoint-1);
        if(modifierContent == NULL)
            return false;
        if(!CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(modifierContent))
        {
            CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
            return false;
        }
        CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
        modifiers = nextBeginPoint;
    }
    wchar_t *endPoint = modifiers+wcslen(modifiers)-1;
    modifierContent = CFASSFileDialogueTextContentOverrideContentCreateWithString(modifiers, endPoint);
    if(modifierContent == NULL)
        return false;
    if(!CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(modifierContent))
    {
        CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
        return false;
    }
    CFASSFileDialogueTextContentOverrideContentDestory(modifierContent);
    return true;
}

static bool CFASSFileDialogueTextContentOverrideContentCheckAnimationModifierSupport(CFASSFileDialogueTextContentOverrideContentRef content)
{
    CFASSFileDialogueTextContentOverrideContentType type = content->type;
    if(type == CFASSFileDialogueTextContentOverrideContentTypeFontSize ||
       type == CFASSFileDialogueTextContentOverrideContentTypeFontSpacing ||
       type == CFASSFileDialogueTextContentOverrideContentTypeColor ||
       type == CFASSFileDialogueTextContentOverrideContentTypeAlpha ||
       type == CFASSFileDialogueTextContentOverrideContentTypeFontScale ||
       type == CFASSFileDialogueTextContentOverrideContentTypeTextRotation ||
       type == CFASSFileDialogueTextContentOverrideContentTypeTextShearing ||
       type == CFASSFileDialogueTextContentOverrideContentTypeBorder ||
       type == CFASSFileDialogueTextContentOverrideContentTypeShadow ||
       type == CFASSFileDialogueTextContentOverrideContentTypeBlurEdge)
        return true;
    else if(type == CFASSFileDialogueTextContentOverrideContentTypeClip &&
            !content->data.clip.usingDrawingCommand)
        return true;
    return false;
}










































