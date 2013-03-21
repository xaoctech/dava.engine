#include "Debug/DVAssertMessage.h"

#if defined (__DAVAENGINE_WIN32__)

namespace DAVA
{

WideString s2ws(const String& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    WideString r(buf);
    delete[] buf;
    return r;
}

void DVAssertMessage::InnerShow(const char* content)
{
	MessageBox(HWND_DESKTOP, s2ws(content).c_str(), L"Assert", MB_OK | MB_ICONEXCLAMATION);
}

};

#endif //#if defined (__DAVAENGINE_WIN32__)

