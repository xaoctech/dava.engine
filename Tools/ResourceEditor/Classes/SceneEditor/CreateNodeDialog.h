#ifndef __CREATE_NODE_DIALOG_H__
#define __CREATE_NODE_DIALOG_H__

#include "DAVAEngine.h"

using namespace DAVA;

class CreateNodeDialogDelegeate
{
public:

    virtual void DialogClosed(int32 retCode) = 0;
};

class CreateNodeDialog: public UIControl
{
    enum eRetCode
    {
        RCODE_CANCEL = 0,
        RCODE_OK,
    };
    
    enum eConst
    {
        BUTTON_HEIGHT = 20,
        BUTTON_WIDTH = 100,
    };
    
public:
    CreateNodeDialog(const Rect & rect);
    virtual ~CreateNodeDialog();
    
    void SetDelegate(CreateNodeDialogDelegeate *delegate);
    
protected:

    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnOk(BaseObject * object, void * userData, void * callerData);

    CreateNodeDialogDelegeate *dialogDelegate;
};



#endif // __CREATE_NODE_DIALOG_H__