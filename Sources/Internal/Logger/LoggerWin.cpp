#include "Logger/Logger.h"

#if defined(__DAVAENGINE_WINDOWS__)

#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN_UAP__)
#include <objbase.h>
#include "Utils/UTF8Utils.h"
#endif

namespace DAVA
{
void Logger::PlatformLog(eLogLevel ll, const char8* text)
{
    ::OutputDebugStringA(text);

#if defined(__DAVAENGINE_WIN_UAP__)
    {
        static Windows::Foundation::Diagnostics::LoggingChannel ^ lc = []() {
            GUID rawguid;
            // {4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a} is GUID of "Microsoft-Windows-Diagnostics-LoggingChannel"
            // Details: https://docs.microsoft.com/en-us/uwp/api/windows.foundation.diagnostics.loggingchannel#remarks
            HRESULT hr = IIDFromString(L"{4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a}", &rawguid);
            DVASSERT(SUCCEEDED(hr));
            return ref new Windows::Foundation::Diagnostics::LoggingChannel("DAVALogProvider", nullptr, Platform::Guid(rawguid));
        }();

        Windows::Foundation::Diagnostics::LoggingLevel lv;

        switch (ll)
        {
        case DAVA::Logger::LEVEL_FRAMEWORK:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Verbose;
            break;
        case DAVA::Logger::LEVEL_DEBUG:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Verbose;
            break;
        case DAVA::Logger::LEVEL_INFO:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Information;
            break;
        case DAVA::Logger::LEVEL_WARNING:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Warning;
            break;
        case DAVA::Logger::LEVEL_ERROR:
            lv = Windows::Foundation::Diagnostics::LoggingLevel::Error;
            break;

        case DAVA::Logger::LEVEL__DISABLE:
        default:
            return;
        }

        WideString wtext = UTF8Utils::EncodeToWideString(text);
        // Platform::StringReference should prevent an extra copy here.
        // Details: https://docs.microsoft.com/en-us/cpp/cppcx/strings-c-cx#stringreference
        lc->LogMessage(Platform::StringReference(wtext.c_str(), wtext.size()), lv);
    }
#endif
}
}
#endif
