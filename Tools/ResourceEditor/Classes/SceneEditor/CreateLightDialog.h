#ifndef __CREATE_LIGHT_DIALOG_H__
#define __CREATE_LIGHT_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateLightDialog: public CreateNodeDialog
{
    
public:
    CreateLightDialog(const Rect & rect);
    virtual ~CreateLightDialog();
    
protected:

    virtual void InitializeProperties();
    virtual void CreateNode();
    virtual void ClearPropertyValues();

};



#endif // __CREATE_LIGHT_DIALOG_H__