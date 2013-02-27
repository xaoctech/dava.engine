#include "Debug/DVAssertMessage.h"

using namespace DAVA;

#if defined (ENABLE_ASSERT_MESSAGE)


void DVAssertMessage::ShowMessage(const char8 * text, ...)
{
	va_list vl;
	va_start(vl, text);
	
	char tmp[4096] = {0};
	// sizeof(tmp) - 2  - We need two characters for appending "\n" if the number of characters exceeds the size of buffer. 
	vsnprintf(tmp, sizeof(tmp)-2, text, vl);
	strcat(tmp, "\n");

	InnerShow(tmp);

	va_end(vl);
}


#else

void DVAssertMessage::ShowMessage(const char8 * /*text*/, ...)
{
	// Do nothing here.
}

#endif	// ENABLE_ASSERT_MESSAGE




