#ifndef __CREATE_SERVICENODE_DIALOG_H__
#define __CREATE_SERVICENODE_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateServiceNodeDialog: public CreateNodeDialog
{
    
public:
    CreateServiceNodeDialog(const Rect & rect);
    
protected:

    virtual void CreateNode();

};



#endif // __CREATE_SERVICENODE_DIALOG_H__