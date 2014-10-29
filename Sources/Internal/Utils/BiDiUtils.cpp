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

#include "BiDiUtils.h"
#include "Utils/UTF8Utils.h"
#include "fribidi/fribidi.h"

namespace DAVA
{

static const uint32 MAX_LINE_LENGTH = 65535;

bool BiDiUtils::TransformString(WideString const& input, WideString& output, bool& isRTL)
{
    FriBidiParType base = FRIBIDI_PAR_ON; // Detect string type automaticaly (LTR,RTL...)
    String inputUTF = UTF8Utils::EncodeToUTF8(input);
    uint32 utf_len = (uint32)inputUTF.length();
    uint32 fribidi_len;
    char outstring[MAX_LINE_LENGTH];

    FriBidiChar* logical =  new FriBidiChar[utf_len + 1];
    fribidi_len = fribidi_charset_to_unicode (FRIBIDI_CHAR_SET_UTF8, inputUTF.c_str(), utf_len, logical);
    
    FriBidiChar* visual = new FriBidiChar[fribidi_len + 1];
    FriBidiLevel log2vis = fribidi_log2vis (logical, fribidi_len, &base, visual, NULL, NULL, NULL);

    if(log2vis != 0)
    {
        fribidi_len = fribidi_remove_bidi_marks (visual, fribidi_len, NULL, NULL, NULL);
        utf_len = fribidi_unicode_to_charset (FRIBIDI_CHAR_SET_UTF8, visual, fribidi_len, outstring);
        UTF8Utils::EncodeToWideString(reinterpret_cast<unsigned char*>(outstring), utf_len, output);
        isRTL = FRIBIDI_IS_RTL(base);
    }
    
    SAFE_DELETE(logical);
    SAFE_DELETE(visual);
    return log2vis != 0;
}

}
