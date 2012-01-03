#ifndef __CREATE_CAMERA_DIALOG_H__
#define __CREATE_CAMERA_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateCameraDialog: public CreateNodeDialog
{
    
public:
    CreateCameraDialog(const Rect & rect);
    
protected:

    virtual void CreateNode();
};



#endif // __CREATE_CAMERA_DIALOG_H__