#ifndef __CREATE_SERVICENODE_DIALOG_H__
#define __CREATE_SERVICENODE_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateServiceNodeDialog: public CreateNodeDialog
{
    
public:
    CreateServiceNodeDialog(const Rect & rect);
    virtual ~CreateServiceNodeDialog();
    
protected:

    virtual void InitializeProperties();
    virtual void CreateNode();
    virtual void ClearPropertyValues();

};



#endif // __CREATE_SERVICENODE_DIALOG_H__