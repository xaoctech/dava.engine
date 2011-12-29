#ifndef __CREATE_NODE_DIALOG_H__
#define __CREATE_NODE_DIALOG_H__

#include "DAVAEngine.h"
#include "DraggableDialog.h"

using namespace DAVA;

class CreateNodeDialogDelegeate
{
public:

    virtual void DialogClosed(int32 retCode) = 0;
};

class NodePropertyControl;
class CreateNodeDialog: public DraggableDialog
{
    
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
    
    void SetScene(Scene *_scene);
    SceneNode *GetSceneNode();
    
    void SetProjectPath(const String &path);
    
protected:

    virtual void CreateNode();
    void ClearPropertyValues();

    
    void SetHeader(const WideString &headerText);
    
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnOk(BaseObject * object, void * userData, void * callerData);

    CreateNodeDialogDelegeate *dialogDelegate;
    
    Rect propertyRect;
    NodePropertyControl *propertyList;
  
    UIStaticText *header;
    SceneNode *sceneNode;
    Scene *scene;
    
    String projectPath;
};



#endif // __CREATE_NODE_DIALOG_H__