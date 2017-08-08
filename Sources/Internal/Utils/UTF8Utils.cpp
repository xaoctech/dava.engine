#include "Base/Exception.h"
#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"

#include <utf8.h>

namespace DAVA
{

#ifdef __DAVAENGINE_WINDOWS__
static_assert(sizeof(char16) == 2, "check size of wchar_t on current platform");
#else
static_assert(sizeof(char16) == 4, "check size of wchar_t on current platform");
#endif

void UTF8Utils::EncodeToWideString(const uint8* string, size_t size, WideString& result)
{
    DVASSERT(nullptr != string);
    result.clear();
    result.reserve(size); // minimum they will be same

    try
    {
#ifdef __DAVAENGINE_WINDOWS__
        utf8::utf8to16(string, string + size, std::back_inserter(result));
#else
        utf8::utf8to32(string, string + size, std::back_inserter(result));
#endif
    }
    catch (const utf8::exception& exception)
    {
        String msg = "UTF8->WideString Conversion error: " + String(exception.what());
        Logger::Warning(msg.c_str());
        DAVA_THROW(Exception, msg);
    }
};

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
    String result;
    result.reserve(wstring.size()); // minimum they will be same

    try
    {
#ifdef __DAVAENGINE_WINDOWS__
        utf8::utf16to8(wstring.begin(), wstring.end(), std::back_inserter(result));
#else
        utf8::utf32to8(wstring.begin(), wstring.end(), std::back_inserter(result));
#endif
    }
    catch (const utf8::exception& exception)
    {
        String msg = "WideString->UTF8 Conversion error: " + String(exception.what());
        Logger::Warning(msg.c_str());
        DAVA_THROW(Exception, msg);
    }

    return result;
};

String UTF8Utils::Trim(const String& str)
{
    return UTF8Utils::TrimLeft(UTF8Utils::TrimRight(str));
}

String UTF8Utils::TrimLeft(const String& str)
{
    auto begin = str.begin();
    auto end = str.end();
    auto it = begin;
    try
    {
        while (it != end)
        {
            begin = it;
            if (!std::iswspace(utf8::next(it, end)))
            {
                return String(begin, end);
            }
        }
    }
    catch (const utf8::exception& e)
    {
        String msg = "UTF8 Trim begin error: " + String(e.what());
        Logger::Warning(msg.c_str());
        DAVA_THROW(Exception, msg);
    }
    return str;
}

String UTF8Utils::TrimRight(const String& str)
{
    auto begin = str.begin();
    auto end = str.end();
    auto it = end;
    try
    {
        while (it != begin)
        {
            end = it;
            if (!std::iswspace(utf8::prior(it, begin)))
            {
                return String(begin, end);
            }
        }
    }
    catch (const utf8::exception& e)
    {
        String msg = "UTF8 Trim end error: " + String(e.what());
        Logger::Warning(msg.c_str());
        DAVA_THROW(Exception, msg);
    }
    return str;
}

} // namespace DAVA
