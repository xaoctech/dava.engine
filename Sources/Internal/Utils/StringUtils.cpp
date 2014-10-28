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

#include  "StringUtils.h"

#include "../../../Libs/libunibreak/src/linebreak.h"

namespace DAVA
{

void StringUtils::GetLineBreaks(const WideString& string, Vector<eLineBreakType>& breaks, const char8* locale)
{
    breaks.resize(string.length(), LB_NOBREAK); // By default all characters not breakable
    set_linebreaks_utf16(reinterpret_cast<const utf16_t*>(string.c_str()), string.length(), locale, reinterpret_cast<char*>(&breaks.front()));
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

}
