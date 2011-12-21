#ifndef __CREATE_NODE_DIALOG_H__
#define __CREATE_NODE_DIALOG_H__

#include "DAVAEngine.h"
#include "PropertyList.h"


using namespace DAVA;

class CreateNodeDialogDelegeate
{
public:

    virtual void DialogClosed(int32 retCode) = 0;
};

struct NodeDescription
{
    int32 type;
    Vector<PropertyCellData *> properties;
};


class CreateNodeDialog: public UIControl, public PropertyListDelegate
{
    enum eConst
    {
        BUTTON_HEIGHT = 20,
        BUTTON_WIDTH = 100,
    };
    
public:
    enum eRetCode
    {
        RCODE_CANCEL = 0,
        RCODE_OK,
    };
    
public:
    CreateNodeDialog(const Rect & rect);
    virtual ~CreateNodeDialog();
    
    void SetDelegate(CreateNodeDialogDelegeate *delegate);
    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    
    
    void SetProperties(NodeDescription *description);
    
protected:

    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnOk(BaseObject * object, void * userData, void * callerData);

    CreateNodeDialogDelegeate *dialogDelegate;
    
    PropertyList *properties;
    
    NodeDescription *currentDescription;
};



#endif // __CREATE_NODE_DIALOG_H__