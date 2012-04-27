#ifndef __ERROR_NOTIFIER_H__
#define __ERROR_NOTIFIER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ErrorDialog;
class ErrorNotifier: public Singleton<ErrorNotifier>
{
    
public:
    ErrorNotifier();
    virtual ~ErrorNotifier();

    void ShowError(const String &errorMessage);
    void ShowError(const Set<String> &errorMessages);
    
protected:

    ErrorDialog *errorDialog;
};



#endif // __ERROR_NOTIFIER_H__