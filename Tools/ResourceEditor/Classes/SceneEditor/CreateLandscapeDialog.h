#ifndef __CREATE_LANDSCAPE_DIALOG_H__
#define __CREATE_LANDSCAPE_DIALOG_H__

#include "DAVAEngine.h"
#include "CreateNodeDialog.h"


using namespace DAVA;

class CreateLandscapeDialog: public CreateNodeDialog
{
    
public:
    CreateLandscapeDialog(const Rect & rect);
    
protected:

    virtual void CreateNode();

};



#endif // __CREATE_LANDSCAPE_DIALOG_H__