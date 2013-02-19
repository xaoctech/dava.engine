//
//  MailSender.h
//  Framework
//
//  Created by Denis Bespalov on 2/18/13.
//
//

#ifndef Framework_MailSender_h
#define Framework_MailSender_h

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

namespace DAVA 
{
class MailSender : public Singleton <MailSender>
{
public:
	MailSender();
	virtual ~MailSender();
	// Main
	bool SendEmail(const String& email, const String& subject, const String& messageText);
};
};


#endif
