//
//  MailSender.cpp
//  Framework
//
//  Created by Denis Bespalov on 2/18/13.
//
//

#include "Network/MailSender.h"
#include <stdio.h>
#include <stdlib.h> 

namespace DAVA
{

MailSender::MailSender()
{
}

MailSender::~MailSender()
{	

}
#if defined(__DAVAENGINE_WIN32__)
bool MailSender::SendEmail(const String& email, const String& subject, const String& messageText)
{

//#include <stdio.h>      /* printf */
//#include <stdlib.h>     /* system, NULL, EXIT_FAILURE */

  int i;
  printf ("Checking if processor is available...");
  if (system(NULL)) puts ("Ok");
    else exit (EXIT_FAILURE);
 // printf ("Executing command DIR...\n");
  i=system("mailto:mavisson@yandex.ru");
  printf ("The value returned was: %d.\n",i);
  //Runtime.exec();
  return false;
}
#endif

}