#ifndef __ERROR_DIALOG_H__
#define __ERROR_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"

using namespace DAVA;

class ErrorDialog: public ExtendedDialog, public UIListDelegate
{
public:
    ErrorDialog();
    virtual ~ErrorDialog();

    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);

    void Show(const Set<String> &newErrorMessages);
    
protected:

    virtual const Rect DialogRect();
    void OnCancel(BaseObject * owner, void * userData, void * callerData);

    void RecreateListControl();
    
    Set<String> errorMessages;
    UIButton *closeButton;
    UIList *errorList;
};



#endif // __ERROR_DIALOG_H__