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
    
    virtual void WillAppear();
    
    void SetDelegate(CreateNodeDialogDelegeate *delegate);
    
//    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
//    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
//    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
//    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    
    
    void SetScene(Scene *_scene);
    SceneNode *GetSceneNode();
    
    void SetProjectPath(const String &path);
    
protected:

    virtual void InitializeProperties() = 0;
    virtual void CreateNode() = 0;
    virtual void ClearPropertyValues() = 0;

    
    void SetHeader(const WideString &headerText);
    
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnOk(BaseObject * object, void * userData, void * callerData);

    CreateNodeDialogDelegeate *dialogDelegate;
    
    PropertyList *propertyList;
  
    UIStaticText *header;
    SceneNode *sceneNode;
    Scene *scene;
    
    String projectPath;
};



#endif // __CREATE_NODE_DIALOG_H__