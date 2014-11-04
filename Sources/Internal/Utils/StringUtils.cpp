/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DAVAEngine.h"
#include "StringUtils.h"
#include "Utils/UTF8Utils.h"

#include "fribidi/fribidi.h"
#include "unibreak/linebreak.h"

namespace DAVA
{

void StringUtils::GetLineBreaks(const WideString& string, Vector<uint8>& breaks, const char8* locale)
{
    breaks.resize(string.length(), LB_NOBREAK); // By default all characters not breakable
#if defined(__DAVAENGINE_WIN32__) // sizeof(wchar_t) == 2
    set_linebreaks_utf16(reinterpret_cast<const utf16_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#else
    set_linebreaks_utf32(reinterpret_cast<const utf32_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
#endif
}

void StringUtils::Trim(WideString& string)
{
    WideString::iterator it = string.begin();
    WideString::iterator end = string.end();
    WideString::reverse_iterator rit = string.rbegin();
    while (it != end && iswspace(*it)) ++it;
    while (rit.base() != it && iswspace(*rit)) ++rit;
    string.erase(it, rit.base());
}

void StringUtils::TrimLeft(WideString& string)
{
    WideString::iterator it = string.begin();
    WideString::iterator end = string.end();
    while (it != end && iswspace(*it)) ++it;
    string.erase(string.begin(), it);
}

void StringUtils::TrimRight(WideString& string)
{
    WideString::reverse_iterator it = string.rbegin();
    WideString::reverse_iterator end = string.rend();
    while (it != end && iswspace(*it)) ++it;
    string.erase(it.base(), string.end());
}

bool StringUtils::BiDiPrepare(const WideString& logicalStr, WideString& shapeStr, WideString& visualStr, sBiDiParams& params, bool* isRTL)
{
    static const uint32 MAX_LINE_LENGTH = 65535;
    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    String inputUTF = UTF8Utils::EncodeToUTF8(logicalStr);
    uint32 utf_len = (uint32)inputUTF.length();
    uint32 fribidi_len;
    char outstring[MAX_LINE_LENGTH];

    FriBidiChar* logical = new FriBidiChar[utf_len + 1];
    fribidi_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, logical);
    if (fribidi_len == 0)
    {
        SAFE_DELETE(logical);
        return false;
    }
    logical[fribidi_len] = 0;

    DVASSERT(!params.bidi_types);
    params.bidi_types = new FriBidiCharType[fribidi_len];
    if (!params.bidi_types)
    {
        SAFE_DELETE(logical);
        return false;
    }

    FriBidiChar* visual = new FriBidiChar[fribidi_len + 1];
    fribidi_get_bidi_types(logical, fribidi_len, params.bidi_types);
    //params.bidi_types[fribidi_len - 1] = params.bidi_types[fribidi_len - 2];

    DVASSERT(!params.embedding_levels);
    params.embedding_levels = new FriBidiLevel[fribidi_len];
    if (!params.embedding_levels)
    {
        SAFE_DELETE(logical);
        SAFE_DELETE(visual);
        return false;
    }

    FriBidiLevel max_level = fribidi_get_par_embedding_levels(params.bidi_types, fribidi_len, &params.base_dir, params.embedding_levels) - 1;
    if (max_level < 0)
    {
        SAFE_DELETE(logical);
        SAFE_DELETE(visual);
        return false;
    }

    Memcpy(visual, logical, fribidi_len * sizeof(*visual));

    /* Arabic joining */
    FriBidiArabicProp * ar_props = new FriBidiArabicProp[fribidi_len];
    fribidi_get_joining_types(logical, fribidi_len, ar_props);
    fribidi_join_arabic(params.bidi_types, fribidi_len, params.embedding_levels, ar_props);
    fribidi_shape(flags, params.embedding_levels, fribidi_len, ar_props, visual);
    SAFE_DELETE(ar_props);
    
    //fribidi_len = fribidi_remove_bidi_marks(visual, fribidi_len, 0, 0, 0);
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, shapeStr);
    if (isRTL)
    {
        *isRTL = FRIBIDI_IS_RTL(params.base_dir);
    }

    /* line breaking goes here, but we assume one line in this function */
    /* and this should be called once per line, but again, we assume one
    * line in this deprecated function */
    fribidi_boolean status = fribidi_reorder_line(flags, params.bidi_types, fribidi_len, 0, params.base_dir, params.embedding_levels, visual, NULL);

    fribidi_len = fribidi_remove_bidi_marks(visual, fribidi_len, NULL, NULL, NULL);
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, visualStr);

    SAFE_DELETE(logical);
    SAFE_DELETE(visual);
    return (status != 0) && (max_level + 1 > 0);
}

bool StringUtils::BiDiReorder(WideString& string, sBiDiParams& params, uint32 lineOffset, uint32 lineLength)
{
    static const uint32 MAX_LINE_LENGTH = 65535;
    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    String inputUTF = UTF8Utils::EncodeToUTF8(string);
    uint32 utf_len = (uint32)inputUTF.length();
    uint32 fribidi_len;
    char outstring[MAX_LINE_LENGTH];

    FriBidiChar* visual = new FriBidiChar[utf_len + 1];
    fribidi_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, visual);
    if (fribidi_len == 0)
    {
        SAFE_DELETE(visual);
        return false;
    }

    /* line breaking goes here, but we assume one line in this function */
    /* and this should be called once per line, but again, we assume one
    * line in this deprecated function */
    fribidi_boolean status = fribidi_reorder_line(flags, &params.bidi_types[lineOffset], fribidi_len, 0, params.base_dir, &params.embedding_levels[lineOffset], visual, NULL);

    fribidi_len = fribidi_remove_bidi_marks(visual, fribidi_len, 0, 0, 0);
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, string);

    SAFE_DELETE(visual);
    return status != 0;
}

bool StringUtils::BiDiTransform(WideString& string, bool& isRTL, Vector<int32>* logical2virtual, Vector<int32>* virtual2logical)
{
    static const uint32 MAX_LINE_LENGTH = 65535;

    FriBidiParType base = FRIBIDI_PAR_ON; // Detect string type automaticaly (LTR,RTL...)
    String inputUTF = UTF8Utils::EncodeToUTF8(string);
    uint32 utf_len = (uint32)inputUTF.length();
    uint32 fribidi_len;
    char outstring[MAX_LINE_LENGTH];
    
    FriBidiChar* logical = new FriBidiChar[utf_len + 1];
    fribidi_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, logical);

    FriBidiChar* visual = new FriBidiChar[fribidi_len + 1];
    FriBidiStrIndex* l2v = new FriBidiStrIndex[fribidi_len];
    FriBidiStrIndex* v2l = new FriBidiStrIndex[fribidi_len];
    FriBidiLevel log2vis = fribidi_log2vis(logical, fribidi_len, &base, visual, l2v, v2l, 0);

    if (log2vis != 0)
    {
        if (logical2virtual)
        {
            logical2virtual->clear();
            logical2virtual->assign(l2v, l2v + fribidi_len);
        }

        if (virtual2logical)
        {
            virtual2logical->clear();
            virtual2logical->assign(v2l, v2l + fribidi_len);
        }

        fribidi_len = fribidi_remove_bidi_marks(visual, fribidi_len, 0, 0, 0);
        utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
        UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, string);
        isRTL = FRIBIDI_IS_RTL(base);
    }

    SAFE_DELETE(logical);
    SAFE_DELETE(visual);
    SAFE_DELETE(l2v);
    SAFE_DELETE(v2l);
    return log2vis != 0;
}

bool StringUtils::BiDiTransformEx(WideString const& logicalStr, WideString& shapedStr, WideString& visualStr, bool& isRTL, Vector<int32>* logical2virtual, Vector<int32>* virtual2logical)
{
    sBiDiParams params;
    static const uint32 MAX_LINE_LENGTH = 65535;
    static FriBidiFlags flags = FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC;

    String inputUTF = UTF8Utils::EncodeToUTF8(logicalStr);
    uint32 utf_len = (uint32)inputUTF.length();
    uint32 fribidi_len;
    char outstring[MAX_LINE_LENGTH];

    FriBidiChar* logical = new FriBidiChar[utf_len + 1];
    fribidi_len = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, logical);
    if (fribidi_len == 0)
    {
        SAFE_DELETE(logical);
        return false;
    }

    DVASSERT(!params.bidi_types);
    params.bidi_types = new FriBidiCharType[fribidi_len];
    if (!params.bidi_types)
    {
        SAFE_DELETE(logical);
        return false;
    }

    FriBidiChar* visual = new FriBidiChar[fribidi_len + 1];
    fribidi_get_bidi_types(logical, fribidi_len, params.bidi_types);

    DVASSERT(!params.embedding_levels);
    params.embedding_levels = new FriBidiLevel[fribidi_len];
    if (!params.embedding_levels)
    {
        SAFE_DELETE(logical);
        SAFE_DELETE(visual);
        return false;
    }

    FriBidiLevel max_level = fribidi_get_par_embedding_levels(params.bidi_types, fribidi_len, &params.base_dir, params.embedding_levels) - 1;
    if (max_level < 0)
    {
        SAFE_DELETE(logical);
        SAFE_DELETE(visual);
        return false;
    }

    Memcpy(visual, logical, fribidi_len * sizeof(*visual));

    /* Arabic joining */
    FriBidiArabicProp * ar_props = new FriBidiArabicProp[fribidi_len];
    fribidi_get_joining_types(logical, fribidi_len, ar_props);
    fribidi_join_arabic(params.bidi_types, fribidi_len, params.embedding_levels, ar_props);
    fribidi_shape(flags, params.embedding_levels, fribidi_len, ar_props, visual);
    SAFE_DELETE(ar_props);

//    FriBidiChar* shaped = new FriBidiChar[fribidi_len + 1];
//    Memcpy(shaped, visual, (fribidi_len + 1) * sizeof FriBidiChar);

    //uint32 fribidi_len2 = fribidi_remove_bidi_marks(shaped, fribidi_len, 0, 0, 0);
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, shapedStr);

    /* line breaking goes here, but we assume one line in this function */
    /* and this should be called once per line, but again, we assume one
    * line in this deprecated function */
    FriBidiStrIndex* v2l = new FriBidiStrIndex[fribidi_len];
    for (uint32 i = 0; i < fribidi_len; i++)
    {
        v2l[i] = i;
    }

    fribidi_boolean status = fribidi_reorder_line(flags, params.bidi_types, fribidi_len, 0, params.base_dir, params.embedding_levels, visual, v2l);

    FriBidiStrIndex* l2v = new FriBidiStrIndex[fribidi_len];
    for (uint32 i = 0; i < fribidi_len; i++)
    {
        l2v[v2l[i]] = i;
    }

    if (logical2virtual)
    {
        logical2virtual->clear();
        logical2virtual->assign(l2v, l2v + fribidi_len);
    }

    if (virtual2logical)
    {
        virtual2logical->clear();
        virtual2logical->assign(v2l, v2l + fribidi_len);
    }

    isRTL = FRIBIDI_IS_RTL(params.base_dir);

    fribidi_len = fribidi_remove_bidi_marks(visual, fribidi_len, l2v, v2l, 0);
    utf_len = fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
    UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, visualStr);

    SAFE_DELETE(logical);
    SAFE_DELETE(visual);
    SAFE_DELETE(v2l);
    SAFE_DELETE(l2v);

    return (status != 0) && (max_level + 1 != 0);
}

}