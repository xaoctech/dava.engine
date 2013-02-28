//
//  MailSender.h
//  
//
//  Created by Denis Bespalov on 2/18/13.
//
//

#ifndef __DAVAENGINE_MAILSENDER_H__
#define __DAVAENGINE_MAILSENDER_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{
class MailSender
{
public:
	static bool SendEmail(const WideString& email, const WideString& subject, const WideString& messageText);
};
};


#endif
