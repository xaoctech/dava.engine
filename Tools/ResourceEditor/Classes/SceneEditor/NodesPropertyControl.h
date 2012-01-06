#ifndef __NODES_PROPERTY_CONTROL_H__
#define __NODES_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

using namespace DAVA;


class NodesPropertyDelegate
{
public:
    
    virtual void NodesPropertyChanged() = 0;
    
};

class NodesPropertyControl: public UIControl, public PropertyListDelegate
{
    
public:
    NodesPropertyControl(const Rect & rect, bool createNodeProperties);
    virtual ~NodesPropertyControl();
    
    virtual void WillAppear();

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void WriteTo(SceneNode *sceneNode);

    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    virtual void OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4);

    void SetDelegate(NodesPropertyDelegate *delegate);

protected:

    bool IsValidPath(const String &path);

    void UpdateProjectPath();
    
    Vector<String> types;
    Vector<String> renderingModes;
    
    NodesPropertyDelegate *nodesDelegate;
    PropertyList *propertyList;
    
    String projectPath;
    
    bool createNodeProperties;
};



#endif // __NODES_PROPERTY_CONTROL_H__