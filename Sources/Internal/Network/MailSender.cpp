//
//  MailSender.cpp
//  
//
//  Created by Denis Bespalov on 2/18/13.
//
//

#include "Network/MailSender.h"
#if defined(__DAVAENGINE_WIN32__)
#include "Utils/StringFormat.h"
#include <shellapi.h>

using namespace DAVA;

bool MailSender::SendEmail(const WideString& email, const WideString& subject, const WideString& messageText)
{
	// Don't try to send email if recipient is not set
	if (email.empty() || email == L"")
		return false;
	WideString mailtoText = L"";
	
	// Create mailto text
	mailtoText = Format(L"mailto:?to=%s&subject=%s&body=%s", email.c_str(), subject.c_str(), messageText.c_str());
	// Call default mail client
	int result = (int)ShellExecute(NULL, L"open", mailtoText.c_str(), NULL, NULL, SW_SHOWNORMAL);
	
	// If the function succeeds, it returns a value greater than 32.
	// If the function fails, it returns an error value that indicates the cause of the failure. 
	return (result > 32);
}

#endif