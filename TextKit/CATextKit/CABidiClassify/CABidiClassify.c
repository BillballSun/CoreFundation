//
//  CABidiClassify.c
//  TextKit
//
//  Created by Bill Sun on 2018/10/2.
//  Copyright © 2018 Bill Sun. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "CFException.h"
#include "CABidiClassify.h"
#include "inttypes.h"

#define LOOP for(;;)

typedef struct CABidiRange {
    CABidiType type;
    UTF32Char from, to;
}CABidiRange;

static CABidiRange *CABidiTypeDatabase = NULL;  /* if database loaded, this is not NULL */
static unsigned long databaseSize = 0lu;

static void CABidiClassifyLoadDefaultDatabase(void);
static void CABidiClassifyLoadDatabaseFromString(const char *string);
static void CABidiClassifyUnloadDatabase(void);
static CABidiRange *CABidiClassifyFindRangeForCharacter(UTF32Char ch);

#pragma mark - Query for Character Type & Category

CABidiType CABidiClassifyTypeForCharacter(UTF32Char character)
{
    if(CABidiTypeDatabase == NULL) CABidiClassifyLoadDefaultDatabase();
    if(CABidiTypeDatabase != NULL)
    {
        CABidiRange *range_p = CABidiClassifyFindRangeForCharacter(character);
        if(range_p != NULL)
            return range_p->type;
    }
    return CABidiTypeL;
}

CABidiCategory CABidiClassifyCategoryForCharacter(UTF32Char character)
{
    CABidiType type = CABidiClassifyTypeForCharacter(character);
    switch (type) {
        case CABidiTypeL: case CABidiTypeR: case CABidiTypeAL:
            return CABidiCategoryStrong;
        case CABidiTypeEN: case CABidiTypeES: case CABidiTypeET: case CABidiTypeAN: case CABidiTypeCS: case CABidiTypeNSM: case CABidiTypeBN:
            return CABidiCategoryWeak;
        case CABidiTypeB: case CABidiTypeS: case CABidiTypeWS: case CABidiTypeON:
            return CABidiCategoryNeutral;
        case CABidiTypeLRE: case CABidiTypeLRO: case CABidiTypeRLE: case CABidiTypeRLO: case CABidiTypePDF: case CABidiTypeLRI: case CABidiTypeRLI: case CABidiTypeFSI: case CABidiTypePDI:
            return CABidiCategoryExplicitFormatting;
    }
}

CABidiCategory CABidiClassifyCategoryForType(CABidiType type)
{
    switch (type) {
        case CABidiTypeL: case CABidiTypeR: case CABidiTypeAL:
            return CABidiCategoryStrong;
        case CABidiTypeEN: case CABidiTypeES: case CABidiTypeET: case CABidiTypeAN: case CABidiTypeCS: case CABidiTypeNSM: case CABidiTypeBN:
            return CABidiCategoryWeak;
        case CABidiTypeB: case CABidiTypeS: case CABidiTypeWS: case CABidiTypeON:
            return CABidiCategoryNeutral;
        case CABidiTypeLRE: case CABidiTypeLRO: case CABidiTypeRLE: case CABidiTypeRLO: case CABidiTypePDF: case CABidiTypeLRI: case CABidiTypeRLI: case CABidiTypeFSI: case CABidiTypePDI:
            return CABidiCategoryExplicitFormatting;
    }
}

bool isExplicitDirectionalIsolates(CABidiType type)
{
    if(type == CABidiTypeLRI || type == CABidiTypeRLI || type == CABidiTypeFSI)
        return true;
    return false;
}

bool isExplicitDirectionalEmbeddings(CABidiType type)
{
    if(type == CABidiTypeLRE || type == CABidiTypeLRO || type == CABidiTypeRLE || type == CABidiTypeRLO)
        return true;
    return false;
}

#pragma mark - Internal Database

static const char *default_derivedBidiClass =

/* code point assigned, version Unicode 11.0 UDC - DerivedBidiClass.txt */
"0041..005a    ; L\n"
"0061..007a    ; L\n"
"00aa          ; L\n"
"00b5          ; L\n"
"00ba          ; L\n"
"00c0..00d6    ; L\n"
"00d8..00f6    ; L\n"
"00f8..02b8    ; L\n"
"02bb..02c1    ; L\n"
"02d0..02d1    ; L\n"
"02e0..02e4    ; L\n"
"02ee          ; L\n"
"0370..0373    ; L\n"
"0376..0377    ; L\n"
"037a..037d    ; L\n"
"037f          ; L\n"
"0386          ; L\n"
"0388..038a    ; L\n"
"038c          ; L\n"
"038e..03a1    ; L\n"
"03a3..03f5    ; L\n"
"03f7..0482    ; L\n"
"048a..052f    ; L\n"
"0531..0556    ; L\n"
"0559..055f    ; L\n"
"0561..0587    ; L\n"
"0589          ; L\n"
"0903..0939    ; L\n"
"093b          ; L\n"
"093d..0940    ; L\n"
"0949..094c    ; L\n"
"094e..0950    ; L\n"
"0958..0961    ; L\n"
"0964..0980    ; L\n"
"0982..0983    ; L\n"
"0985..098c    ; L\n"
"098f..0990    ; L\n"
"0993..09a8    ; L\n"
"09aa..09b0    ; L\n"
"09b2          ; L\n"
"09b6..09b9    ; L\n"
"09bd..09c0    ; L\n"
"09c7..09c8    ; L\n"
"09cb..09cc    ; L\n"
"09ce          ; L\n"
"09d7          ; L\n"
"09dc..09dd    ; L\n"
"09df..09e1    ; L\n"
"09e6..09f1    ; L\n"
"09f4..09fa    ; L\n"
"0a03          ; L\n"
"0a05..0a0a    ; L\n"
"0a0f..0a10    ; L\n"
"0a13..0a28    ; L\n"
"0a2a..0a30    ; L\n"
"0a32..0a33    ; L\n"
"0a35..0a36    ; L\n"
"0a38..0a39    ; L\n"
"0a3e..0a40    ; L\n"
"0a59..0a5c    ; L\n"
"0a5e          ; L\n"
"0a66..0a6f    ; L\n"
"0a72..0a74    ; L\n"
"0a83          ; L\n"
"0a85..0a8d    ; L\n"
"0a8f..0a91    ; L\n"
"0a93..0aa8    ; L\n"
"0aaa..0ab0    ; L\n"
"0ab2..0ab3    ; L\n"
"0ab5..0ab9    ; L\n"
"0abd..0ac0    ; L\n"
"0ac9          ; L\n"
"0acb..0acc    ; L\n"
"0ad0          ; L\n"
"0ae0..0ae1    ; L\n"
"0ae6..0af0    ; L\n"
"0af9          ; L\n"
"0b02..0b03    ; L\n"
"0b05..0b0c    ; L\n"
"0b0f..0b10    ; L\n"
"0b13..0b28    ; L\n"
"0b2a..0b30    ; L\n"
"0b32..0b33    ; L\n"
"0b35..0b39    ; L\n"
"0b3d..0b3e    ; L\n"
"0b40          ; L\n"
"0b47..0b48    ; L\n"
"0b4b..0b4c    ; L\n"
"0b57          ; L\n"
"0b5c..0b5d    ; L\n"
"0b5f..0b61    ; L\n"
"0b66..0b77    ; L\n"
"0b83          ; L\n"
"0b85..0b8a    ; L\n"
"0b8e..0b90    ; L\n"
"0b92..0b95    ; L\n"
"0b99..0b9a    ; L\n"
"0b9c          ; L\n"
"0b9e..0b9f    ; L\n"
"0ba3..0ba4    ; L\n"
"0ba8..0baa    ; L\n"
"0bae..0bb9    ; L\n"
"0bbe..0bbf    ; L\n"
"0bc1..0bc2    ; L\n"
"0bc6..0bc8    ; L\n"
"0bca..0bcc    ; L\n"
"0bd0          ; L\n"
"0bd7          ; L\n"
"0be6..0bf2    ; L\n"
"0c01..0c03    ; L\n"
"0c05..0c0c    ; L\n"
"0c0e..0c10    ; L\n"
"0c12..0c28    ; L\n"
"0c2a..0c39    ; L\n"
"0c3d          ; L\n"
"0c41..0c44    ; L\n"
"0c58..0c5a    ; L\n"
"0c60..0c61    ; L\n"
"0c66..0c6f    ; L\n"
"0c7f          ; L\n"
"0c82..0c83    ; L\n"
"0c85..0c8c    ; L\n"
"0c8e..0c90    ; L\n"
"0c92..0ca8    ; L\n"
"0caa..0cb3    ; L\n"
"0cb5..0cb9    ; L\n"
"0cbd..0cc4    ; L\n"
"0cc6..0cc8    ; L\n"
"0cca..0ccb    ; L\n"
"0cd5..0cd6    ; L\n"
"0cde          ; L\n"
"0ce0..0ce1    ; L\n"
"0ce6..0cef    ; L\n"
"0cf1..0cf2    ; L\n"
"0d02..0d03    ; L\n"
"0d05..0d0c    ; L\n"
"0d0e..0d10    ; L\n"
"0d12..0d3a    ; L\n"
"0d3d..0d40    ; L\n"
"0d46..0d48    ; L\n"
"0d4a..0d4c    ; L\n"
"0d4e          ; L\n"
"0d57          ; L\n"
"0d5f..0d61    ; L\n"
"0d66..0d75    ; L\n"
"0d79..0d7f    ; L\n"
"0d82..0d83    ; L\n"
"0d85..0d96    ; L\n"
"0d9a..0db1    ; L\n"
"0db3..0dbb    ; L\n"
"0dbd          ; L\n"
"0dc0..0dc6    ; L\n"
"0dcf..0dd1    ; L\n"
"0dd8..0ddf    ; L\n"
"0de6..0def    ; L\n"
"0df2..0df4    ; L\n"
"0e01..0e30    ; L\n"
"0e32..0e33    ; L\n"
"0e40..0e46    ; L\n"
"0e4f..0e5b    ; L\n"
"0e81..0e82    ; L\n"
"0e84          ; L\n"
"0e87..0e88    ; L\n"
"0e8a          ; L\n"
"0e8d          ; L\n"
"0e94..0e97    ; L\n"
"0e99..0e9f    ; L\n"
"0ea1..0ea3    ; L\n"
"0ea5          ; L\n"
"0ea7          ; L\n"
"0eaa..0eab    ; L\n"
"0ead..0eb0    ; L\n"
"0eb2..0eb3    ; L\n"
"0ebd          ; L\n"
"0ec0..0ec4    ; L\n"
"0ec6          ; L\n"
"0ed0..0ed9    ; L\n"
"0edc..0edf    ; L\n"
"0f00..0f17    ; L\n"
"0f1a..0f34    ; L\n"
"0f36          ; L\n"
"0f38          ; L\n"
"0f3e..0f47    ; L\n"
"0f49..0f6c    ; L\n"
"0f7f          ; L\n"
"0f85          ; L\n"
"0f88..0f8c    ; L\n"
"0fbe..0fc5    ; L\n"
"0fc7..0fcc    ; L\n"
"0fce..0fda    ; L\n"
"1000..102c    ; L\n"
"1031          ; L\n"
"1038          ; L\n"
"103b..103c    ; L\n"
"103f..1057    ; L\n"
"105a..105d    ; L\n"
"1061..1070    ; L\n"
"1075..1081    ; L\n"
"1083..1084    ; L\n"
"1087..108c    ; L\n"
"108e..109c    ; L\n"
"109e..10c5    ; L\n"
"10c7          ; L\n"
"10cd          ; L\n"
"10d0..1248    ; L\n"
"124a..124d    ; L\n"
"1250..1256    ; L\n"
"1258          ; L\n"
"125a..125d    ; L\n"
"1260..1288    ; L\n"
"128a..128d    ; L\n"
"1290..12b0    ; L\n"
"12b2..12b5    ; L\n"
"12b8..12be    ; L\n"
"12c0          ; L\n"
"12c2..12c5    ; L\n"
"12c8..12d6    ; L\n"
"12d8..1310    ; L\n"
"1312..1315    ; L\n"
"1318..135a    ; L\n"
"1360..137c    ; L\n"
"1380..138f    ; L\n"
"13a0..13f5    ; L\n"
"13f8..13fd    ; L\n"
"1401..167f    ; L\n"
"1681..169a    ; L\n"
"16a0..16f8    ; L\n"
"1700..170c    ; L\n"
"170e..1711    ; L\n"
"1720..1731    ; L\n"
"1735..1736    ; L\n"
"1740..1751    ; L\n"
"1760..176c    ; L\n"
"176e..1770    ; L\n"
"1780..17b3    ; L\n"
"17b6          ; L\n"
"17be..17c5    ; L\n"
"17c7..17c8    ; L\n"
"17d4..17da    ; L\n"
"17dc          ; L\n"
"17e0..17e9    ; L\n"
"1810..1819    ; L\n"
"1820..1877    ; L\n"
"1880..18a8    ; L\n"
"18aa          ; L\n"
"18b0..18f5    ; L\n"
"1900..191e    ; L\n"
"1923..1926    ; L\n"
"1929..192b    ; L\n"
"1930..1931    ; L\n"
"1933..1938    ; L\n"
"1946..196d    ; L\n"
"1970..1974    ; L\n"
"1980..19ab    ; L\n"
"19b0..19c9    ; L\n"
"19d0..19da    ; L\n"
"1a00..1a16    ; L\n"
"1a19..1a1a    ; L\n"
"1a1e..1a55    ; L\n"
"1a57          ; L\n"
"1a61          ; L\n"
"1a63..1a64    ; L\n"
"1a6d..1a72    ; L\n"
"1a80..1a89    ; L\n"
"1a90..1a99    ; L\n"
"1aa0..1aad    ; L\n"
"1b04..1b33    ; L\n"
"1b35          ; L\n"
"1b3b          ; L\n"
"1b3d..1b41    ; L\n"
"1b43..1b4b    ; L\n"
"1b50..1b6a    ; L\n"
"1b74..1b7c    ; L\n"
"1b82..1ba1    ; L\n"
"1ba6..1ba7    ; L\n"
"1baa          ; L\n"
"1bae..1be5    ; L\n"
"1be7          ; L\n"
"1bea..1bec    ; L\n"
"1bee          ; L\n"
"1bf2..1bf3    ; L\n"
"1bfc..1c2b    ; L\n"
"1c34..1c35    ; L\n"
"1c3b..1c49    ; L\n"
"1c4d..1c7f    ; L\n"
"1cc0..1cc7    ; L\n"
"1cd3          ; L\n"
"1ce1          ; L\n"
"1ce9..1cec    ; L\n"
"1cee..1cf3    ; L\n"
"1cf5..1cf6    ; L\n"
"1d00..1dbf    ; L\n"
"1e00..1f15    ; L\n"
"1f18..1f1d    ; L\n"
"1f20..1f45    ; L\n"
"1f48..1f4d    ; L\n"
"1f50..1f57    ; L\n"
"1f59          ; L\n"
"1f5b          ; L\n"
"1f5d          ; L\n"
"1f5f..1f7d    ; L\n"
"1f80..1fb4    ; L\n"
"1fb6..1fbc    ; L\n"
"1fbe          ; L\n"
"1fc2..1fc4    ; L\n"
"1fc6..1fcc    ; L\n"
"1fd0..1fd3    ; L\n"
"1fd6..1fdb    ; L\n"
"1fe0..1fec    ; L\n"
"1ff2..1ff4    ; L\n"
"1ff6..1ffc    ; L\n"
"200e          ; L\n"
"2071          ; L\n"
"207f          ; L\n"
"2090..209c    ; L\n"
"2102          ; L\n"
"2107          ; L\n"
"210a..2113    ; L\n"
"2115          ; L\n"
"2119..211d    ; L\n"
"2124          ; L\n"
"2126          ; L\n"
"2128          ; L\n"
"212a..212d    ; L\n"
"212f..2139    ; L\n"
"213c..213f    ; L\n"
"2145..2149    ; L\n"
"214e..214f    ; L\n"
"2160..2188    ; L\n"
"2336..237a    ; L\n"
"2395          ; L\n"
"249c..24e9    ; L\n"
"26ac          ; L\n"
"2800..28ff    ; L\n"
"2c00..2c2e    ; L\n"
"2c30..2c5e    ; L\n"
"2c60..2ce4    ; L\n"
"2ceb..2cee    ; L\n"
"2cf2..2cf3    ; L\n"
"2d00..2d25    ; L\n"
"2d27          ; L\n"
"2d2d          ; L\n"
"2d30..2d67    ; L\n"
"2d6f..2d70    ; L\n"
"2d80..2d96    ; L\n"
"2da0..2da6    ; L\n"
"2da8..2dae    ; L\n"
"2db0..2db6    ; L\n"
"2db8..2dbe    ; L\n"
"2dc0..2dc6    ; L\n"
"2dc8..2dce    ; L\n"
"2dd0..2dd6    ; L\n"
"2dd8..2dde    ; L\n"
"3005..3007    ; L\n"
"3021..3029    ; L\n"
"302e..302f    ; L\n"
"3031..3035    ; L\n"
"3038..303c    ; L\n"
"3041..3096    ; L\n"
"309d..309f    ; L\n"
"30a1..30fa    ; L\n"
"30fc..30ff    ; L\n"
"3105..312d    ; L\n"
"3131..318e    ; L\n"
"3190..31ba    ; L\n"
"31f0..321c    ; L\n"
"3220..324f    ; L\n"
"3260..327b    ; L\n"
"327f..32b0    ; L\n"
"32c0..32cb    ; L\n"
"32d0..32fe    ; L\n"
"3300..3376    ; L\n"
"337b..33dd    ; L\n"
"33e0..33fe    ; L\n"
"3400..4db5    ; L\n"
"4e00..9fd5    ; L\n"
"a000..a48c    ; L\n"
"a4d0..a60c    ; L\n"
"a610..a62b    ; L\n"
"a640..a66e    ; L\n"
"a680..a69d    ; L\n"
"a6a0..a6ef    ; L\n"
"a6f2..a6f7    ; L\n"
"a722..a787    ; L\n"
"a789..a7ad    ; L\n"
"a7b0..a7b7    ; L\n"
"a7f7..a801    ; L\n"
"a803..a805    ; L\n"
"a807..a80a    ; L\n"
"a80c..a824    ; L\n"
"a827          ; L\n"
"a830..a837    ; L\n"
"a840..a873    ; L\n"
"a880..a8c3    ; L\n"
"a8ce..a8d9    ; L\n"
"a8f2..a8fd    ; L\n"
"a900..a925    ; L\n"
"a92e..a946    ; L\n"
"a952..a953    ; L\n"
"a95f..a97c    ; L\n"
"a983..a9b2    ; L\n"
"a9b4..a9b5    ; L\n"
"a9ba..a9bb    ; L\n"
"a9bd..a9cd    ; L\n"
"a9cf..a9d9    ; L\n"
"a9de..a9e4    ; L\n"
"a9e6..a9fe    ; L\n"
"aa00..aa28    ; L\n"
"aa2f..aa30    ; L\n"
"aa33..aa34    ; L\n"
"aa40..aa42    ; L\n"
"aa44..aa4b    ; L\n"
"aa4d          ; L\n"
"aa50..aa59    ; L\n"
"aa5c..aa7b    ; L\n"
"aa7d..aaaf    ; L\n"
"aab1          ; L\n"
"aab5..aab6    ; L\n"
"aab9..aabd    ; L\n"
"aac0          ; L\n"
"aac2          ; L\n"
"aadb..aaeb    ; L\n"
"aaee..aaf5    ; L\n"
"ab01..ab06    ; L\n"
"ab09..ab0e    ; L\n"
"ab11..ab16    ; L\n"
"ab20..ab26    ; L\n"
"ab28..ab2e    ; L\n"
"ab30..ab65    ; L\n"
"ab70..abe4    ; L\n"
"abe6..abe7    ; L\n"
"abe9..abec    ; L\n"
"abf0..abf9    ; L\n"
"ac00..d7a3    ; L\n"
"d7b0..d7c6    ; L\n"
"d7cb..d7fb    ; L\n"
"e000..fa6d    ; L\n"
"fa70..fad9    ; L\n"
"fb00..fb06    ; L\n"
"fb13..fb17    ; L\n"
"ff21..ff3a    ; L\n"
"ff41..ff5a    ; L\n"
"ff66..ffbe    ; L\n"
"ffc2..ffc7    ; L\n"
"ffca..ffcf    ; L\n"
"ffd2..ffd7    ; L\n"
"ffda..ffdc    ; L\n"
"10000..1000b  ; L\n"
"1000d..10026  ; L\n"
"10028..1003a  ; L\n"
"1003c..1003d  ; L\n"
"1003f..1004d  ; L\n"
"10050..1005d  ; L\n"
"10080..100fa  ; L\n"
"10100         ; L\n"
"10102         ; L\n"
"10107..10133  ; L\n"
"10137..1013f  ; L\n"
"101d0..101fc  ; L\n"
"10280..1029c  ; L\n"
"102a0..102d0  ; L\n"
"10300..10323  ; L\n"
"10330..1034a  ; L\n"
"10350..10375  ; L\n"
"10380..1039d  ; L\n"
"1039f..103c3  ; L\n"
"103c8..103d5  ; L\n"
"10400..1049d  ; L\n"
"104a0..104a9  ; L\n"
"10500..10527  ; L\n"
"10530..10563  ; L\n"
"1056f         ; L\n"
"10600..10736  ; L\n"
"10740..10755  ; L\n"
"10760..10767  ; L\n"
"11000         ; L\n"
"11002..11037  ; L\n"
"11047..1104d  ; L\n"
"11066..1106f  ; L\n"
"11082..110b2  ; L\n"
"110b7..110b8  ; L\n"
"110bb..110c1  ; L\n"
"110d0..110e8  ; L\n"
"110f0..110f9  ; L\n"
"11103..11126  ; L\n"
"1112c         ; L\n"
"11136..11143  ; L\n"
"11150..11172  ; L\n"
"11174..11176  ; L\n"
"11182..111b5  ; L\n"
"111bf..111c9  ; L\n"
"111cd         ; L\n"
"111d0..111df  ; L\n"
"111e1..111f4  ; L\n"
"11200..11211  ; L\n"
"11213..1122e  ; L\n"
"11232..11233  ; L\n"
"11235         ; L\n"
"11238..1123d  ; L\n"
"11280..11286  ; L\n"
"11288         ; L\n"
"1128a..1128d  ; L\n"
"1128f..1129d  ; L\n"
"1129f..112a9  ; L\n"
"112b0..112de  ; L\n"
"112e0..112e2  ; L\n"
"112f0..112f9  ; L\n"
"11302..11303  ; L\n"
"11305..1130c  ; L\n"
"1130f..11310  ; L\n"
"11313..11328  ; L\n"
"1132a..11330  ; L\n"
"11332..11333  ; L\n"
"11335..11339  ; L\n"
"1133d..1133f  ; L\n"
"11341..11344  ; L\n"
"11347..11348  ; L\n"
"1134b..1134d  ; L\n"
"11350         ; L\n"
"11357         ; L\n"
"1135d..11363  ; L\n"
"11480..114b2  ; L\n"
"114b9         ; L\n"
"114bb..114be  ; L\n"
"114c1         ; L\n"
"114c4..114c7  ; L\n"
"114d0..114d9  ; L\n"
"11580..115b1  ; L\n"
"115b8..115bb  ; L\n"
"115be         ; L\n"
"115c1..115db  ; L\n"
"11600..11632  ; L\n"
"1163b..1163c  ; L\n"
"1163e         ; L\n"
"11641..11644  ; L\n"
"11650..11659  ; L\n"
"11680..116aa  ; L\n"
"116ac         ; L\n"
"116ae..116af  ; L\n"
"116b6         ; L\n"
"116c0..116c9  ; L\n"
"11700..11719  ; L\n"
"11720..11721  ; L\n"
"11726         ; L\n"
"11730..1173f  ; L\n"
"118a0..118f2  ; L\n"
"118ff         ; L\n"
"11ac0..11af8  ; L\n"
"12000..12399  ; L\n"
"12400..1246e  ; L\n"
"12470..12474  ; L\n"
"12480..12543  ; L\n"
"13000..1342e  ; L\n"
"14400..14646  ; L\n"
"16800..16a38  ; L\n"
"16a40..16a5e  ; L\n"
"16a60..16a69  ; L\n"
"16a6e..16a6f  ; L\n"
"16ad0..16aed  ; L\n"
"16af5         ; L\n"
"16b00..16b2f  ; L\n"
"16b37..16b45  ; L\n"
"16b50..16b59  ; L\n"
"16b5b..16b61  ; L\n"
"16b63..16b77  ; L\n"
"16b7d..16b8f  ; L\n"
"16f00..16f44  ; L\n"
"16f50..16f7e  ; L\n"
"16f93..16f9f  ; L\n"
"1b000..1b001  ; L\n"
"1bc00..1bc6a  ; L\n"
"1bc70..1bc7c  ; L\n"
"1bc80..1bc88  ; L\n"
"1bc90..1bc99  ; L\n"
"1bc9c         ; L\n"
"1bc9f         ; L\n"
"1d000..1d0f5  ; L\n"
"1d100..1d126  ; L\n"
"1d129..1d166  ; L\n"
"1d16a..1d172  ; L\n"
"1d183..1d184  ; L\n"
"1d18c..1d1a9  ; L\n"
"1d1ae..1d1e8  ; L\n"
"1d360..1d371  ; L\n"
"1d400..1d454  ; L\n"
"1d456..1d49c  ; L\n"
"1d49e..1d49f  ; L\n"
"1d4a2         ; L\n"
"1d4a5..1d4a6  ; L\n"
"1d4a9..1d4ac  ; L\n"
"1d4ae..1d4b9  ; L\n"
"1d4bb         ; L\n"
"1d4bd..1d4c3  ; L\n"
"1d4c5..1d505  ; L\n"
"1d507..1d50a  ; L\n"
"1d50d..1d514  ; L\n"
"1d516..1d51c  ; L\n"
"1d51e..1d539  ; L\n"
"1d53b..1d53e  ; L\n"
"1d540..1d544  ; L\n"
"1d546         ; L\n"
"1d54a..1d550  ; L\n"
"1d552..1d6a5  ; L\n"
"1d6a8..1d6da  ; L\n"
"1d6dc..1d714  ; L\n"
"1d716..1d74e  ; L\n"
"1d750..1d788  ; L\n"
"1d78a..1d7c2  ; L\n"
"1d7c4..1d7cb  ; L\n"
"1d800..1d9ff  ; L\n"
"1da37..1da3a  ; L\n"
"1da6d..1da74  ; L\n"
"1da76..1da83  ; L\n"
"1da85..1da8b  ; L\n"
"1f110..1f12e  ; L\n"
"1f130..1f169  ; L\n"
"1f170..1f19a  ; L\n"
"1f1e6..1f202  ; L\n"
"1f210..1f23a  ; L\n"
"1f240..1f248  ; L\n"
"1f250..1f251  ; L\n"
"20000..2a6d6  ; L\n"
"2a700..2b734  ; L\n"
"2b740..2b81d  ; L\n"
"2b820..2cea1  ; L\n"
"2f800..2fa1d  ; L\n"
"f0000..ffffd  ; L\n"
"100000..10fffd; L\n"
"0590          ; R\n"
"05be          ; R\n"
"05c0          ; R\n"
"05c3          ; R\n"
"05c6          ; R\n"
"05c8..05ff    ; R\n"
"07c0..07ea    ; R\n"
"07f4..07f5    ; R\n"
"07fa..0815    ; R\n"
"081a          ; R\n"
"0824          ; R\n"
"0828          ; R\n"
"082e..0858    ; R\n"
"085c..089f    ; R\n"
"200f          ; R\n"
"fb1d          ; R\n"
"fb1f..fb28    ; R\n"
"fb2a..fb4f    ; R\n"
"10800..1091e  ; R\n"
"10920..10a00  ; R\n"
"10a04         ; R\n"
"10a07..10a0b  ; R\n"
"10a10..10a37  ; R\n"
"10a3b..10a3e  ; R\n"
"10a40..10ae4  ; R\n"
"10ae7..10b38  ; R\n"
"10b40..10e5f  ; R\n"
"10e7f..10fff  ; R\n"
"1e800..1e8cf  ; R\n"
"1e8d7..1edff  ; R\n"
"1ef00..1efff  ; R\n"
"0030..0039    ; EN\n"
"00b2..00b3    ; EN\n"
"00b9          ; EN\n"
"06f0..06f9    ; EN\n"
"2070          ; EN\n"
"2074..2079    ; EN\n"
"2080..2089    ; EN\n"
"2488..249b    ; EN\n"
"ff10..ff19    ; EN\n"
"102e1..102fb  ; EN\n"
"1d7ce..1d7ff  ; EN\n"
"1f100..1f10a  ; EN\n"
"002b          ; ES\n"
"002d          ; ES\n"
"207a..207b    ; ES\n"
"208a..208b    ; ES\n"
"2212          ; ES\n"
"fb29          ; ES\n"
"fe62..fe63    ; ES\n"
"ff0b          ; ES\n"
"ff0d          ; ES\n"
"0023..0025    ; ET\n"
"00a2..00a5    ; ET\n"
"00b0..00b1    ; ET\n"
"058f          ; ET\n"
"0609..060a    ; ET\n"
"066a          ; ET\n"
"09f2..09f3    ; ET\n"
"09fb          ; ET\n"
"0af1          ; ET\n"
"0bf9          ; ET\n"
"0e3f          ; ET\n"
"17db          ; ET\n"
"2030..2034    ; ET\n"
"20a0..20cf    ; ET\n"
"212e          ; ET\n"
"2213          ; ET\n"
"a838..a839    ; ET\n"
"fe5f          ; ET\n"
"fe69..fe6a    ; ET\n"
"ff03..ff05    ; ET\n"
"ffe0..ffe1    ; ET\n"
"ffe5..ffe6    ; ET\n"
"0600..0605    ; AN\n"
"0660..0669    ; AN\n"
"066b..066c    ; AN\n"
"06dd          ; AN\n"
"10e60..10e7e  ; AN\n"
"002c          ; CS\n"
"002e..002f    ; CS\n"
"003a          ; CS\n"
"00a0          ; CS\n"
"060c          ; CS\n"
"202f          ; CS\n"
"2044          ; CS\n"
"fe50          ; CS\n"
"fe52          ; CS\n"
"fe55          ; CS\n"
"ff0c          ; CS\n"
"ff0e..ff0f    ; CS\n"
"ff1a          ; CS\n"
"000a          ; B\n"
"000d          ; B\n"
"001c..001e    ; B\n"
"0085          ; B\n"
"2029          ; B\n"
"0009          ; S\n"
"000b          ; S\n"
"001f          ; S\n"
"000c          ; WS\n"
"0020          ; WS\n"
"1680          ; WS\n"
"2000..200a    ; WS\n"
"2028          ; WS\n"
"205f          ; WS\n"
"3000          ; WS\n"
"0021..0022    ; ON\n"
"0026..002a    ; ON\n"
"003b..0040    ; ON\n"
"005b..0060    ; ON\n"
"007b..007e    ; ON\n"
"00a1          ; ON\n"
"00a6..00a9    ; ON\n"
"00ab..00ac    ; ON\n"
"00ae..00af    ; ON\n"
"00b4          ; ON\n"
"00b6..00b8    ; ON\n"
"00bb..00bf    ; ON\n"
"00d7          ; ON\n"
"00f7          ; ON\n"
"02b9..02ba    ; ON\n"
"02c2..02cf    ; ON\n"
"02d2..02df    ; ON\n"
"02e5..02ed    ; ON\n"
"02ef..02ff    ; ON\n"
"0374..0375    ; ON\n"
"037e          ; ON\n"
"0384..0385    ; ON\n"
"0387          ; ON\n"
"03f6          ; ON\n"
"058a          ; ON\n"
"058d..058e    ; ON\n"
"0606..0607    ; ON\n"
"060e..060f    ; ON\n"
"06de          ; ON\n"
"06e9          ; ON\n"
"07f6..07f9    ; ON\n"
"0bf3..0bf8    ; ON\n"
"0bfa          ; ON\n"
"0c78..0c7e    ; ON\n"
"0f3a..0f3d    ; ON\n"
"1390..1399    ; ON\n"
"1400          ; ON\n"
"169b..169c    ; ON\n"
"17f0..17f9    ; ON\n"
"1800..180a    ; ON\n"
"1940          ; ON\n"
"1944..1945    ; ON\n"
"19de..19ff    ; ON\n"
"1fbd          ; ON\n"
"1fbf..1fc1    ; ON\n"
"1fcd..1fcf    ; ON\n"
"1fdd..1fdf    ; ON\n"
"1fed..1fef    ; ON\n"
"1ffd..1ffe    ; ON\n"
"2010..2027    ; ON\n"
"2035..2043    ; ON\n"
"2045..205e    ; ON\n"
"207c..207e    ; ON\n"
"208c..208e    ; ON\n"
"2100..2101    ; ON\n"
"2103..2106    ; ON\n"
"2108..2109    ; ON\n"
"2114          ; ON\n"
"2116..2118    ; ON\n"
"211e..2123    ; ON\n"
"2125          ; ON\n"
"2127          ; ON\n"
"2129          ; ON\n"
"213a..213b    ; ON\n"
"2140..2144    ; ON\n"
"214a..214d    ; ON\n"
"2150..215f    ; ON\n"
"2189..218b    ; ON\n"
"2190..2211    ; ON\n"
"2214..2335    ; ON\n"
"237b..2394    ; ON\n"
"2396..23fa    ; ON\n"
"2400..2426    ; ON\n"
"2440..244a    ; ON\n"
"2460..2487    ; ON\n"
"24ea..26ab    ; ON\n"
"26ad..27ff    ; ON\n"
"2900..2b73    ; ON\n"
"2b76..2b95    ; ON\n"
"2b98..2bb9    ; ON\n"
"2bbd..2bc8    ; ON\n"
"2bca..2bd1    ; ON\n"
"2bec..2bef    ; ON\n"
"2ce5..2cea    ; ON\n"
"2cf9..2cff    ; ON\n"
"2e00..2e42    ; ON\n"
"2e80..2e99    ; ON\n"
"2e9b..2ef3    ; ON\n"
"2f00..2fd5    ; ON\n"
"2ff0..2ffb    ; ON\n"
"3001..3004    ; ON\n"
"3008..3020    ; ON\n"
"3030          ; ON\n"
"3036..3037    ; ON\n"
"303d..303f    ; ON\n"
"309b..309c    ; ON\n"
"30a0          ; ON\n"
"30fb          ; ON\n"
"31c0..31e3    ; ON\n"
"321d..321e    ; ON\n"
"3250..325f    ; ON\n"
"327c..327e    ; ON\n"
"32b1..32bf    ; ON\n"
"32cc..32cf    ; ON\n"
"3377..337a    ; ON\n"
"33de..33df    ; ON\n"
"33ff          ; ON\n"
"4dc0..4dff    ; ON\n"
"a490..a4c6    ; ON\n"
"a60d..a60f    ; ON\n"
"a673          ; ON\n"
"a67e..a67f    ; ON\n"
"a700..a721    ; ON\n"
"a788          ; ON\n"
"a828..a82b    ; ON\n"
"a874..a877    ; ON\n"
"fd3e..fd3f    ; ON\n"
"fdfd          ; ON\n"
"fe10..fe19    ; ON\n"
"fe30..fe4f    ; ON\n"
"fe51          ; ON\n"
"fe54          ; ON\n"
"fe56..fe5e    ; ON\n"
"fe60..fe61    ; ON\n"
"fe64..fe66    ; ON\n"
"fe68          ; ON\n"
"fe6b          ; ON\n"
"ff01..ff02    ; ON\n"
"ff06..ff0a    ; ON\n"
"ff1b..ff20    ; ON\n"
"ff3b..ff40    ; ON\n"
"ff5b..ff65    ; ON\n"
"ffe2..ffe4    ; ON\n"
"ffe8..ffee    ; ON\n"
"fff9..fffd    ; ON\n"
"10101         ; ON\n"
"10140..1018c  ; ON\n"
"10190..1019b  ; ON\n"
"101a0         ; ON\n"
"1091f         ; ON\n"
"10b39..10b3f  ; ON\n"
"11052..11065  ; ON\n"
"1d200..1d241  ; ON\n"
"1d245         ; ON\n"
"1d300..1d356  ; ON\n"
"1d6db         ; ON\n"
"1d715         ; ON\n"
"1d74f         ; ON\n"
"1d789         ; ON\n"
"1d7c3         ; ON\n"
"1eef0..1eef1  ; ON\n"
"1f000..1f02b  ; ON\n"
"1f030..1f093  ; ON\n"
"1f0a0..1f0ae  ; ON\n"
"1f0b1..1f0bf  ; ON\n"
"1f0c1..1f0cf  ; ON\n"
"1f0d1..1f0f5  ; ON\n"
"1f10b..1f10c  ; ON\n"
"1f16a..1f16b  ; ON\n"
"1f300..1f579  ; ON\n"
"1f57b..1f5a3  ; ON\n"
"1f5a5..1f6d0  ; ON\n"
"1f6e0..1f6ec  ; ON\n"
"1f6f0..1f6f3  ; ON\n"
"1f700..1f773  ; ON\n"
"1f780..1f7d4  ; ON\n"
"1f800..1f80b  ; ON\n"
"1f810..1f847  ; ON\n"
"1f850..1f859  ; ON\n"
"1f860..1f887  ; ON\n"
"1f890..1f8ad  ; ON\n"
"1f910..1f918  ; ON\n"
"1f980..1f984  ; ON\n"
"1f9c0         ; ON\n"
"0000..0008    ; BN\n"
"000e..001b    ; BN\n"
"007f..0084    ; BN\n"
"0086..009f    ; BN\n"
"00ad          ; BN\n"
"180e          ; BN\n"
"200b..200d    ; BN\n"
"2060..2065    ; BN\n"
"206a..206f    ; BN\n"
"fdd0..fdef    ; BN\n"
"feff          ; BN\n"
"fff0..fff8    ; BN\n"
"fffe..ffff    ; BN\n"
"1bca0..1bca3  ; BN\n"
"1d173..1d17a  ; BN\n"
"1fffe..1ffff  ; BN\n"
"2fffe..2ffff  ; BN\n"
"3fffe..3ffff  ; BN\n"
"4fffe..4ffff  ; BN\n"
"5fffe..5ffff  ; BN\n"
"6fffe..6ffff  ; BN\n"
"7fffe..7ffff  ; BN\n"
"8fffe..8ffff  ; BN\n"
"9fffe..9ffff  ; BN\n"
"afffe..affff  ; BN\n"
"bfffe..bffff  ; BN\n"
"cfffe..cffff  ; BN\n"
"dfffe..e00ff  ; BN\n"
"e01f0..e0fff  ; BN\n"
"efffe..effff  ; BN\n"
"ffffe..fffff  ; BN\n"
"10fffe..10ffff; BN\n"
"0300..036f    ; NSM\n"
"0483..0489    ; NSM\n"
"0591..05bd    ; NSM\n"
"05bf          ; NSM\n"
"05c1..05c2    ; NSM\n"
"05c4..05c5    ; NSM\n"
"05c7          ; NSM\n"
"0610..061a    ; NSM\n"
"064b..065f    ; NSM\n"
"0670          ; NSM\n"
"06d6..06dc    ; NSM\n"
"06df..06e4    ; NSM\n"
"06e7..06e8    ; NSM\n"
"06ea..06ed    ; NSM\n"
"0711          ; NSM\n"
"0730..074a    ; NSM\n"
"07a6..07b0    ; NSM\n"
"07eb..07f3    ; NSM\n"
"0816..0819    ; NSM\n"
"081b..0823    ; NSM\n"
"0825..0827    ; NSM\n"
"0829..082d    ; NSM\n"
"0859..085b    ; NSM\n"
"08e3..0902    ; NSM\n"
"093a          ; NSM\n"
"093c          ; NSM\n"
"0941..0948    ; NSM\n"
"094d          ; NSM\n"
"0951..0957    ; NSM\n"
"0962..0963    ; NSM\n"
"0981          ; NSM\n"
"09bc          ; NSM\n"
"09c1..09c4    ; NSM\n"
"09cd          ; NSM\n"
"09e2..09e3    ; NSM\n"
"0a01..0a02    ; NSM\n"
"0a3c          ; NSM\n"
"0a41..0a42    ; NSM\n"
"0a47..0a48    ; NSM\n"
"0a4b..0a4d    ; NSM\n"
"0a51          ; NSM\n"
"0a70..0a71    ; NSM\n"
"0a75          ; NSM\n"
"0a81..0a82    ; NSM\n"
"0abc          ; NSM\n"
"0ac1..0ac5    ; NSM\n"
"0ac7..0ac8    ; NSM\n"
"0acd          ; NSM\n"
"0ae2..0ae3    ; NSM\n"
"0b01          ; NSM\n"
"0b3c          ; NSM\n"
"0b3f          ; NSM\n"
"0b41..0b44    ; NSM\n"
"0b4d          ; NSM\n"
"0b56          ; NSM\n"
"0b62..0b63    ; NSM\n"
"0b82          ; NSM\n"
"0bc0          ; NSM\n"
"0bcd          ; NSM\n"
"0c00          ; NSM\n"
"0c3e..0c40    ; NSM\n"
"0c46..0c48    ; NSM\n"
"0c4a..0c4d    ; NSM\n"
"0c55..0c56    ; NSM\n"
"0c62..0c63    ; NSM\n"
"0c81          ; NSM\n"
"0cbc          ; NSM\n"
"0ccc..0ccd    ; NSM\n"
"0ce2..0ce3    ; NSM\n"
"0d01          ; NSM\n"
"0d41..0d44    ; NSM\n"
"0d4d          ; NSM\n"
"0d62..0d63    ; NSM\n"
"0dca          ; NSM\n"
"0dd2..0dd4    ; NSM\n"
"0dd6          ; NSM\n"
"0e31          ; NSM\n"
"0e34..0e3a    ; NSM\n"
"0e47..0e4e    ; NSM\n"
"0eb1          ; NSM\n"
"0eb4..0eb9    ; NSM\n"
"0ebb..0ebc    ; NSM\n"
"0ec8..0ecd    ; NSM\n"
"0f18..0f19    ; NSM\n"
"0f35          ; NSM\n"
"0f37          ; NSM\n"
"0f39          ; NSM\n"
"0f71..0f7e    ; NSM\n"
"0f80..0f84    ; NSM\n"
"0f86..0f87    ; NSM\n"
"0f8d..0f97    ; NSM\n"
"0f99..0fbc    ; NSM\n"
"0fc6          ; NSM\n"
"102d..1030    ; NSM\n"
"1032..1037    ; NSM\n"
"1039..103a    ; NSM\n"
"103d..103e    ; NSM\n"
"1058..1059    ; NSM\n"
"105e..1060    ; NSM\n"
"1071..1074    ; NSM\n"
"1082          ; NSM\n"
"1085..1086    ; NSM\n"
"108d          ; NSM\n"
"109d          ; NSM\n"
"135d..135f    ; NSM\n"
"1712..1714    ; NSM\n"
"1732..1734    ; NSM\n"
"1752..1753    ; NSM\n"
"1772..1773    ; NSM\n"
"17b4..17b5    ; NSM\n"
"17b7..17bd    ; NSM\n"
"17c6          ; NSM\n"
"17c9..17d3    ; NSM\n"
"17dd          ; NSM\n"
"180b..180d    ; NSM\n"
"18a9          ; NSM\n"
"1920..1922    ; NSM\n"
"1927..1928    ; NSM\n"
"1932          ; NSM\n"
"1939..193b    ; NSM\n"
"1a17..1a18    ; NSM\n"
"1a1b          ; NSM\n"
"1a56          ; NSM\n"
"1a58..1a5e    ; NSM\n"
"1a60          ; NSM\n"
"1a62          ; NSM\n"
"1a65..1a6c    ; NSM\n"
"1a73..1a7c    ; NSM\n"
"1a7f          ; NSM\n"
"1ab0..1abe    ; NSM\n"
"1b00..1b03    ; NSM\n"
"1b34          ; NSM\n"
"1b36..1b3a    ; NSM\n"
"1b3c          ; NSM\n"
"1b42          ; NSM\n"
"1b6b..1b73    ; NSM\n"
"1b80..1b81    ; NSM\n"
"1ba2..1ba5    ; NSM\n"
"1ba8..1ba9    ; NSM\n"
"1bab..1bad    ; NSM\n"
"1be6          ; NSM\n"
"1be8..1be9    ; NSM\n"
"1bed          ; NSM\n"
"1bef..1bf1    ; NSM\n"
"1c2c..1c33    ; NSM\n"
"1c36..1c37    ; NSM\n"
"1cd0..1cd2    ; NSM\n"
"1cd4..1ce0    ; NSM\n"
"1ce2..1ce8    ; NSM\n"
"1ced          ; NSM\n"
"1cf4          ; NSM\n"
"1cf8..1cf9    ; NSM\n"
"1dc0..1df5    ; NSM\n"
"1dfc..1dff    ; NSM\n"
"20d0..20f0    ; NSM\n"
"2cef..2cf1    ; NSM\n"
"2d7f          ; NSM\n"
"2de0..2dff    ; NSM\n"
"302a..302d    ; NSM\n"
"3099..309a    ; NSM\n"
"a66f..a672    ; NSM\n"
"a674..a67d    ; NSM\n"
"a69e..a69f    ; NSM\n"
"a6f0..a6f1    ; NSM\n"
"a802          ; NSM\n"
"a806          ; NSM\n"
"a80b          ; NSM\n"
"a825..a826    ; NSM\n"
"a8c4          ; NSM\n"
"a8e0..a8f1    ; NSM\n"
"a926..a92d    ; NSM\n"
"a947..a951    ; NSM\n"
"a980..a982    ; NSM\n"
"a9b3          ; NSM\n"
"a9b6..a9b9    ; NSM\n"
"a9bc          ; NSM\n"
"a9e5          ; NSM\n"
"aa29..aa2e    ; NSM\n"
"aa31..aa32    ; NSM\n"
"aa35..aa36    ; NSM\n"
"aa43          ; NSM\n"
"aa4c          ; NSM\n"
"aa7c          ; NSM\n"
"aab0          ; NSM\n"
"aab2..aab4    ; NSM\n"
"aab7..aab8    ; NSM\n"
"aabe..aabf    ; NSM\n"
"aac1          ; NSM\n"
"aaec..aaed    ; NSM\n"
"aaf6          ; NSM\n"
"abe5          ; NSM\n"
"abe8          ; NSM\n"
"abed          ; NSM\n"
"fb1e          ; NSM\n"
"fe00..fe0f    ; NSM\n"
"fe20..fe2f    ; NSM\n"
"101fd         ; NSM\n"
"102e0         ; NSM\n"
"10376..1037a  ; NSM\n"
"10a01..10a03  ; NSM\n"
"10a05..10a06  ; NSM\n"
"10a0c..10a0f  ; NSM\n"
"10a38..10a3a  ; NSM\n"
"10a3f         ; NSM\n"
"10ae5..10ae6  ; NSM\n"
"11001         ; NSM\n"
"11038..11046  ; NSM\n"
"1107f..11081  ; NSM\n"
"110b3..110b6  ; NSM\n"
"110b9..110ba  ; NSM\n"
"11100..11102  ; NSM\n"
"11127..1112b  ; NSM\n"
"1112d..11134  ; NSM\n"
"11173         ; NSM\n"
"11180..11181  ; NSM\n"
"111b6..111be  ; NSM\n"
"111ca..111cc  ; NSM\n"
"1122f..11231  ; NSM\n"
"11234         ; NSM\n"
"11236..11237  ; NSM\n"
"112df         ; NSM\n"
"112e3..112ea  ; NSM\n"
"11300..11301  ; NSM\n"
"1133c         ; NSM\n"
"11340         ; NSM\n"
"11366..1136c  ; NSM\n"
"11370..11374  ; NSM\n"
"114b3..114b8  ; NSM\n"
"114ba         ; NSM\n"
"114bf..114c0  ; NSM\n"
"114c2..114c3  ; NSM\n"
"115b2..115b5  ; NSM\n"
"115bc..115bd  ; NSM\n"
"115bf..115c0  ; NSM\n"
"115dc..115dd  ; NSM\n"
"11633..1163a  ; NSM\n"
"1163d         ; NSM\n"
"1163f..11640  ; NSM\n"
"116ab         ; NSM\n"
"116ad         ; NSM\n"
"116b0..116b5  ; NSM\n"
"116b7         ; NSM\n"
"1171d..1171f  ; NSM\n"
"11722..11725  ; NSM\n"
"11727..1172b  ; NSM\n"
"16af0..16af4  ; NSM\n"
"16b30..16b36  ; NSM\n"
"16f8f..16f92  ; NSM\n"
"1bc9d..1bc9e  ; NSM\n"
"1d167..1d169  ; NSM\n"
"1d17b..1d182  ; NSM\n"
"1d185..1d18b  ; NSM\n"
"1d1aa..1d1ad  ; NSM\n"
"1d242..1d244  ; NSM\n"
"1da00..1da36  ; NSM\n"
"1da3b..1da6c  ; NSM\n"
"1da75         ; NSM\n"
"1da84         ; NSM\n"
"1da9b..1da9f  ; NSM\n"
"1daa1..1daaf  ; NSM\n"
"1e8d0..1e8d6  ; NSM\n"
"e0100..e01ef  ; NSM\n"
"0608          ; AL\n"
"060b          ; AL\n"
"060d          ; AL\n"
"061b..064a    ; AL\n"
"066d..066f    ; AL\n"
"0671..06d5    ; AL\n"
"06e5..06e6    ; AL\n"
"06ee..06ef    ; AL\n"
"06fa..0710    ; AL\n"
"0712..072f    ; AL\n"
"074b..07a5    ; AL\n"
"07b1..07bf    ; AL\n"
"08a0..08e2    ; AL\n"
"fb50..fd3d    ; AL\n"
"fd40..fdcf    ; AL\n"
"fdf0..fdfc    ; AL\n"
"fdfe..fdff    ; AL\n"
"fe70..fefe    ; AL\n"
"1ee00..1eeef  ; AL\n"
"1eef2..1eeff  ; AL\n"
"202d          ; LRO\n"
"202e          ; RLO\n"
"202a          ; LRE\n"
"202b          ; RLE\n"
"202c          ; PDF\n"
"2066          ; LRI\n"
"2067          ; RLI\n"
"2068          ; FSI\n"
"2069          ; PDI\n"

/* unassigned code point for AL */
"0600..06FF    ; AL\n"      // Arabic
"0700..074F    ; AL\n"      // Syriac
"0750..077F    ; AL\n"      // Arabic_Supplement
"0780..07BF    ; AL\n"      // Thaana
"08A0..08FF    ; AL\n"      // Arabic Extended-A
"FB50..FDCF    ; AL\n"      // Arabic_Presentation_Forms_A
"FDF0..FDFF    ; AL\n"      // Arabic_Presentation_Forms_A
"FE70..FEFF    ; AL\n"      // Arabic_Presentation_Forms_B
"1EE00..1EEFF  ; AL\n"    // Arabic Mathematical Alphabetic Symbols

/* unassigned code point for R */
"0590..05FF    ; R\n"       // Hebrew
"07C0..07FF    ; R\n" // NKo
"10800..1083F  ; R\n"   // Cypriot_Syllabary
"10900..1091F  ; R\n"    // Phoenician
"10920..1093F  ; R\n"    // Lydian
"10980..1099F  ; R\n"  // Meroitic Hieroglyphs
"109A0..109FF  ; R\n"  // Meroitic Cursive
"10A00..10A5F  ; R\n"    // Kharoshthi
"0800..089F    ; R\n" // and any others in the ranges
"FB1D..FB4F    ; R\n" // and any others in the ranges
"10840..10FFF  ; R\n" // and any others in the ranges
"1E800..1EDFF  ; R\n" // and any others in the ranges
"1EF00..1EFFF  ; R\n"  // and any others in the ranges

/* unassigned code point for ET */
"20A0..20CF    ; ET\n";

/* for other unassigned, default to L */

#define DatabaseInitializedSize 1232
#define DatabaseIncreaseSize 250
#define MaxBidiClassTypeLength 3

#define Stringlization(x) _Stringlization(x)
#define _Stringlization(x) #x

static unsigned long databaseAllocatedSize = 0lu;

static CABidiType CABidiClassifyTypeFromString(const char *string);
static void CABidiClassifyInsertRangeIntoDatabase(CABidiRange range);
static int CABidiRangeCompareCharacter(UTF32Char ch, CABidiRange range);
static void CABidiClassifyAddRangeFromString(const char *rangeString);

static void CABidiClassifyLoadDefaultDatabase(void)
{
    CABidiClassifyLoadDatabaseFromString(default_derivedBidiClass);
}

static void CABidiClassifyLoadDatabaseFromString(const char *string)
{
    const char *current = string;
    do
    {
        CABidiClassifyAddRangeFromString(current);
        do current++; while(*current!='\n' && *current!='\0');
        if(*current == '\n') current++;
        else break;
    }while (1);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
static void CABidiClassifyUnloadDatabase(void)
{
    if(CABidiTypeDatabase != NULL)
    {
        free(CABidiTypeDatabase);
        CABidiTypeDatabase = NULL;
        databaseAllocatedSize = 0u;
        databaseSize = 0u;
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"

static void CABidiClassifyAddRangeFromString(const char *rangeString)
{
    UTF32Char from, to;
    char typeString[MaxBidiClassTypeLength + 1];
    CABidiType type;
    bool valid = false;
    if(sscanf(rangeString, "%"UTF32CharSCNx".."UTF32CharSCNx" ; %" Stringlization(MaxBidiClassTypeLength) "s", &from, &to, typeString) == 3)
        valid = true;
    else if(sscanf(rangeString, "%"UTF32CharSCNx" ; %" Stringlization(MaxBidiClassTypeLength) "s", &from, typeString) == 2)
    {
        to = from;
        valid = true;
    }
    if(valid)
    {
        type = CABidiClassifyTypeFromString(typeString);
        CABidiRange range = {.type = type, .from = from, .to = to};
        
        CABidiClassifyInsertRangeIntoDatabase(range);
    }
}

#pragma clang diagnostic pop

static void CABidiClassifyInsertRangeIntoDatabase(CABidiRange range)
{
    if(CABidiTypeDatabase == NULL)
    {
        if((CABidiTypeDatabase = malloc(sizeof(CABidiRange) * DatabaseInitializedSize)) != NULL)
        {
            databaseAllocatedSize = DatabaseInitializedSize;
            databaseSize = 0;
        }
    }
    if(CABidiTypeDatabase != NULL)
    {
        unsigned long insertIndex = 0;
        LOOP
        {
            if(insertIndex == databaseSize)
                break;
            if((range.from <= CABidiTypeDatabase[insertIndex].to && range.from >= CABidiTypeDatabase[insertIndex].from) ||
               (range.to >= CABidiTypeDatabase[insertIndex].from && range.to <= CABidiTypeDatabase[insertIndex].to) ||
               (range.to >= CABidiTypeDatabase[insertIndex].to && range.from <= CABidiTypeDatabase[insertIndex].from) ||
               (range.to <= CABidiTypeDatabase[insertIndex].to && range.from >= CABidiTypeDatabase[insertIndex].from))
            {
                CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABidiClassifyInsertRangeIntoDatabase range exist");
                return;
            }
            else if(range.from > CABidiTypeDatabase[insertIndex].to)
                insertIndex++;
            else
                break;
        }
        if(databaseSize >= databaseAllocatedSize)
        {
            CABidiRange *temp = realloc(CABidiTypeDatabase, sizeof(CABidiRange) * (databaseAllocatedSize + DatabaseIncreaseSize));
            if(temp != NULL)
            {
                CABidiTypeDatabase = temp;
                databaseAllocatedSize = databaseAllocatedSize + DatabaseIncreaseSize;
            }
            else
            {
                CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABidiClassifyInsertRangeIntoDatabase data size increased failed");
                return;
            }
        }
        for(size_t index = databaseSize; index > insertIndex; index--)
            CABidiTypeDatabase[index] = CABidiTypeDatabase[index - 1];
        
        CABidiTypeDatabase[insertIndex] = range;
        databaseSize++;
    }
}

static CABidiRange *CABidiClassifyFindRangeForCharacter(UTF32Char ch)
{
    if(CABidiTypeDatabase == NULL)
    {
        CFExceptionRaise(CFExceptionNameProcessFailed, NULL, "CABidiClassify database is not loaded");
        return NULL;
    }
    long beginIndex = 0;
    long endIndex = databaseSize - 1;
    do
    {
        long midIndex = (beginIndex + endIndex) / 2;
        int compare = CABidiRangeCompareCharacter(ch, CABidiTypeDatabase[midIndex]);
        if(compare < 0)
            endIndex = midIndex - 1;
        else if(compare == 0)
            return CABidiTypeDatabase + midIndex;
        else
            beginIndex = midIndex + 1;
    }while(beginIndex <= endIndex);
    return NULL;
}

static int CABidiRangeCompareCharacter(UTF32Char ch, CABidiRange range)
{
    if(ch < range.from) return -1;
    if(ch > range.to) return 1;
    return 0;
}

static CABidiType CABidiClassifyTypeFromString(const char *string)
{
    if(string == NULL)
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABidiClassifyTypeFromString NULL");
        return CABidiTypeL;
    }
    
    if(strcmp(string, "L") == 0)
        return CABidiTypeL;
    else if(strcmp(string, "R") == 0)
        return CABidiTypeR;
    else if(strcmp(string, "AL") == 0)
        return CABidiTypeAL;
    else if(strcmp(string, "EN") == 0)
        return CABidiTypeEN;
    else if(strcmp(string, "ES") == 0)
        return CABidiTypeES;
    else if(strcmp(string, "ET") == 0)
        return CABidiTypeET;
    else if(strcmp(string, "AN") == 0)
        return CABidiTypeAN;
    else if(strcmp(string, "CS") == 0)
        return CABidiTypeCS;
    else if(strcmp(string, "NSM") == 0)
        return CABidiTypeNSM;
    else if(strcmp(string, "BN") == 0)
        return CABidiTypeBN;
    else if(strcmp(string, "B") == 0)
        return CABidiTypeB;
    else if(strcmp(string, "S") == 0)
        return CABidiTypeS;
    else if(strcmp(string, "WS") == 0)
        return CABidiTypeWS;
    else if(strcmp(string, "ON") == 0)
        return CABidiTypeON;
    else if(strcmp(string, "LRE") == 0)
        return CABidiTypeLRE;
    else if(strcmp(string, "LRO") == 0)
        return CABidiTypeLRO;
    else if(strcmp(string, "RLE") == 0)
        return CABidiTypeRLE;
    else if(strcmp(string, "RLO") == 0)
        return CABidiTypeRLO;
    else if(strcmp(string, "PDF") == 0)
        return CABidiTypePDF;
    else if(strcmp(string, "LRI") == 0)
        return CABidiTypeLRI;
    else if(strcmp(string, "RLI") == 0)
        return CABidiTypeRLI;
    else if(strcmp(string, "FSI") == 0)
        return CABidiTypeFSI;
    else if(strcmp(string, "PDI") == 0)
        return CABidiTypePDI;
    else
    {
        CFExceptionRaise(CFExceptionNameInvalidArgument, NULL, "CABidiClassifyTypeFromString invalid \"%s\"", string);
        return CABidiTypeL;
    }
}
