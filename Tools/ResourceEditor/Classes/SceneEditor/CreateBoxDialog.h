#ifndef __CREATE_BOX_DIALOG_H__
#define __CREATE_BOX_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateBoxDialog: public CreateNodeDialog
{
    
public:
    CreateBoxDialog(const Rect & rect);
    virtual ~CreateBoxDialog();
    
protected:

    virtual void InitializeProperties();
    virtual void CreateNode();
    virtual void ClearPropertyValues();

};



#endif // __CREATE_BOX_DIALOG_H__