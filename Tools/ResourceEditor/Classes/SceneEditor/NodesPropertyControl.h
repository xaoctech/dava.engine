#ifndef __NODES_PROPERTY_CONTROL_H__
#define __NODES_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

#include "CreatePropertyControl.h"

using namespace DAVA;


class NodesPropertyDelegate
{
public:
    
    virtual void NodesPropertyChanged() = 0;
    
};

class NodesPropertyControl: public UIControl, public PropertyListDelegate, public CreatePropertyControlDelegate, public UIListDelegate
{
    enum eConst
    {
        CELL_HEIGHT = 20,
    };
    
public:
    NodesPropertyControl(const Rect & rect, bool createNodeProperties);
    virtual ~NodesPropertyControl();
    
    virtual void WillAppear();

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void ReadFrom(DataNode *dataNode);
    virtual void WriteTo(SceneNode *sceneNode);

    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    virtual void OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4);

    void SetDelegate(NodesPropertyDelegate *delegate);
    
    virtual void NodeCreated(bool success);
    
    
    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    void SetWorkingScene(Scene *scene);
    
protected:

    bool IsValidPath(const String &path);

    Vector<String> types;
    Vector<String> renderingModes;
    Vector<Material*> materials;
    Vector<String> materialNames;

    
    NodesPropertyDelegate *nodesDelegate;
    PropertyList *propertyList;
    
    String projectPath;
    
    bool createNodeProperties;
    
    UIButton *btnPlus;
    UIButton *btnMinus;
    
    void OnPlus(BaseObject * object, void * userData, void * callerData);
    void OnMinus(BaseObject * object, void * userData, void * callerData);
    
    CreatePropertyControl *propControl;
    SceneNode *currentNode;
    
    String GetCustomPropertyName(const String &keyName);
    Scene *workingScene;

    
    UIList *deletionList;
    UIControl *listHolder;
    UIButton *btnCancel;
    void OnCancel(BaseObject * object, void * userData, void * callerData);
    
    
    void OnGo2Materials(BaseObject * object, void * userData, void * callerData);
};



#endif // __NODES_PROPERTY_CONTROL_H__