#include "Debug/DVAssertMessage.h"

using namespace DAVA;

WideString s2ws(String& s)
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

#if defined (ENABLE_ASSERT_MESSAGE)
void DVAssertMessage::ShowMessage(const char8 * text, ...)
{

#if defined(__DAVAENGINE_WIN32__)
	va_list vl;
	va_start(vl, text);
	char tmp[4096] = {0};
	// sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer. 
	_vsnprintf(tmp, sizeof(tmp)-2, text, vl);
	strcat(tmp, "\n");
	String str(tmp);
	MessageBox(HWND_DESKTOP,s2ws(str).c_str(),L"Assert",MB_OK | MB_ICONEXCLAMATION);
	
	va_end(vl);
	
#elif defined(__DAVAENGINE_MACOS__)

#endif

#else
void DVAssertMessage::ShowMessage(const char8 * /*text*/, ...)
{
	// Do nothing here.
}
#endif	// ENABLE_ASSERT_MESSAGE
}





