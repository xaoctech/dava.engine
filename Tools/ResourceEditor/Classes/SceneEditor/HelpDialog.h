#ifndef __HELP_DIALOG_H__
#define __HELP_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"

using namespace DAVA;

class HelpDialog: public ExtendedDialog
{
public:
    HelpDialog();
    virtual ~HelpDialog();

    void Show();
    
protected:

    virtual const Rect DialogRect();
    void OnCancel(BaseObject * owner, void * userData, void * callerData);

    void AddHelpText(const WideString &text, float32 & y);

    
    
    UIButton *closeButton;
};



#endif // __ERROR_DIALOG_H__