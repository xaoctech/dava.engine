#ifndef __CREATE_SPHERE_DIALOG_H__
#define __CREATE_SPHERE_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateSphereDialog: public CreateNodeDialog
{
    
public:
    CreateSphereDialog(const Rect & rect);
    virtual ~CreateSphereDialog();
    
protected:

    virtual void InitializeProperties();
    virtual void CreateNode();
    virtual void ClearPropertyValues();

};



#endif // __CREATE_SPHERE_DIALOG_H__