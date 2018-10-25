//
//  CFASSFileDialogueTextContentOverrideContent.h
//  ASS_editor
//
//  Created by Bill Sun on 2018/5/11.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#ifndef CFASSFileDialogueTextContentOverrideContent_h
#define CFASSFileDialogueTextContentOverrideContent_h

#include <stdbool.h>
#include <wchar.h>

#include "CFGeometry.h"
#include "CFASSFileChange.h"

typedef struct CFASSFileDialogueTextContentOverrideContent *CFASSFileDialogueTextContentOverrideContentRef;

typedef enum CFASSFileDialogueTextContentOverrideContentType {
    CFASSFileDialogueTextContentOverrideContentTypeBlod,
    CFASSFileDialogueTextContentOverrideContentTypeItalic,
    CFASSFileDialogueTextContentOverrideContentTypeUnderline,
    CFASSFileDialogueTextContentOverrideContentTypeStrikeOut,
    CFASSFileDialogueTextContentOverrideContentTypeBorder,
    CFASSFileDialogueTextContentOverrideContentTypeShadow,
    CFASSFileDialogueTextContentOverrideContentTypeBlurEdge,        /* include gaussian blur */
    CFASSFileDialogueTextContentOverrideContentTypeFontName,
    CFASSFileDialogueTextContentOverrideContentTypeFontSize,
    CFASSFileDialogueTextContentOverrideContentTypeFontScale,       /* x, y */
    CFASSFileDialogueTextContentOverrideContentTypeFontSpacing,
    CFASSFileDialogueTextContentOverrideContentTypeTextRotation,    /* default, x, y, z */
    CFASSFileDialogueTextContentOverrideContentTypeTextShearing,
    CFASSFileDialogueTextContentOverrideContentTypeTextEncoding,
    CFASSFileDialogueTextContentOverrideContentTypeColor,
    CFASSFileDialogueTextContentOverrideContentTypeAlpha,
    CFASSFileDialogueTextContentOverrideContentTypeAlignment,
    CFASSFileDialogueTextContentOverrideContentTypeKaraokeDuration,
    CFASSFileDialogueTextContentOverrideContentTypeWrapingStyle,
    CFASSFileDialogueTextContentOverrideContentTypeReset,
    CFASSFileDialogueTextContentOverrideContentTypeAnimation,
    CFASSFileDialogueTextContentOverrideContentTypeMove,
    CFASSFileDialogueTextContentOverrideContentTypePosition,
    CFASSFileDialogueTextContentOverrideContentTypeRotationOrigin,
    CFASSFileDialogueTextContentOverrideContentTypeFade,
    CFASSFileDialogueTextContentOverrideContentTypeClip,
    CFASSFileDialogueTextContentOverrideContentTypeDrawing,
    CFASSFileDialogueTextContentOverrideContentTypeBaselineOffset
} CFASSFileDialogueTextContentOverrideContentType;

#pragma mark - Create/Copy/Destory

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBlodContentCreate(unsigned int blod);
/* blod */
// specify 0 or 1 to toggle blod, or the weight of the font

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideItalicContentCreate(bool isItalic);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideUnderlineContentCreate(bool isUnderline);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideStrikeOutContentCreate(bool isStrikeOut);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBorderSizeContentCreate(bool usingComplexed,
                                                                                                           CF2DAxis axis,
                                                                                                           double resolutionPixels);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideShadowContentCreate(bool usingComplexed,
                                                                                                       CF2DAxis axis,
                                                                                                       double depth);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBlurEdgeContentCreate(bool usingGuassian,
                                                                                                         unsigned int strength);
/* blurEdge */
// if isGaussian is false, specify 0 or 1 to toggle blur in blurTimes, or the times of the blur edge effect applyed, this must be integer
// if isGaussian is true, specify more than 0.0 to toggle blur in blurEdge, this could be floating-point number

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontNameContentCreate(wchar_t *fontName);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontSizeContentCreate(unsigned int fontSize);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontScaleContentCreate(CF2DAxis axis,
                                                                                                          unsigned int percentage);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFontSpacingContentCreate(double resolutionPixels);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextRotationContentCreate(bool defineAxis,
                                                                                                             CF3DAxis axis,
                                                                                                             int degrees);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextShearingContentCreate(CF2DAxis axis,
                                                                                                             double factor);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideTextEncodingContentCreate(unsigned int textEncoding);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideColorContentCreate(bool definesWhichColor,
                                                                                                      unsigned int colorCount,
                                                                                                      unsigned char blue,
                                                                                                      unsigned char green,
                                                                                                      unsigned char red);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAlphaContentCreate(bool setAllComponent,
                                                                                                      unsigned int alphaCount,
                                                                                                      unsigned char alpha);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAlignmentContentCreate(bool isLegacyStyle, unsigned int alignmentPosition);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideKaraokeDurationContentCreate(bool isSweep, unsigned int centiSeconds, bool removeBorderAndShadow);
/*
 /k means show the following with primary color for duration, without sweeping effect
 /K & /kf means with sweeping effect
 /ko is similar to /k, but before highlight with primaryColor, border and shadow are removed
 */

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideWrapStyleContentCreate(unsigned int wrapStyle);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideResetContentCreate(bool resetToDefault, wchar_t *styleName);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverridePositionContentCreate(int x, int y);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideMovementContentCreate(int fromX,
                                                                                                         int fromY,
                                                                                                         int toX,
                                                                                                         int toY,
                                                                                                         bool hasTimeControl,
                                                                                                         unsigned int startFromMS,
                                                                                                         unsigned int endFromMS);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideRotationOriginContentCreate(int x, int y);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideFadeContentCreate(bool isComplexed,
                                                                                                     unsigned int fadeInMS,
                                                                                                     unsigned int fadeOutMS,
                                                                                                     unsigned char beginAlpha,
                                                                                                     unsigned char middleAlpha,
                                                                                                     unsigned char endAlpha,
                                                                                                     unsigned int fadeInBeginMS,
                                                                                                     unsigned int fadeInEndMS,
                                                                                                     unsigned int fadeOutBeginMS,
                                                                                                     unsigned int fadeOutEndMS);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideAnimationContentCreate(bool hasTimeOffset,
                                                                                                          unsigned int beginOffsetMS,
                                                                                                          unsigned int endOffsetMS,
                                                                                                          bool hasAcceleration,
                                                                                                          unsigned int acceleration,
                                                                                                          const wchar_t *modifiers);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideClipContentCreate(bool reverse,
                                                                                                     bool usingDrawingCommand,
                                                                                                     bool hasScale,
                                                                                                     int upLeftX, int upLeftY,
                                                                                                     int lowRightX, int lowRightY,
                                                                                                     unsigned int scale,
                                                                                                     wchar_t *drawingCommand);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideDrawingContentCreate(unsigned int drawingMode);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideBaselineOffsetContentCreate(int towardsBottomPixels);

CFASSFileDialogueTextContentOverrideContentRef CFASSFileDialogueTextContentOverrideContentCopy(CFASSFileDialogueTextContentOverrideContentRef overrideContent);

void CFASSFileDialogueTextContentOverrideContentDestory(CFASSFileDialogueTextContentOverrideContentRef overrideContent);

#pragma mark - Receive Change

void CFASSFileDialogueTextContentOverrideContentMakeChange(CFASSFileDialogueTextContentOverrideContentRef overrideContent, CFASSFileChangeRef change);

#endif /* CFASSFileDialogueTextContentOverrideContent_h */



















































