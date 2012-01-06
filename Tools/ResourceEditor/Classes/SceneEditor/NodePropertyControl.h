#ifndef __NODE_PROPERTY_CONTROL_H__
#define __NODE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

using namespace DAVA;


class NodePropertyDelegate
{
public:
    
    virtual void NodePropertyChanged() = 0;
    
};

class NodePropertyControl: public UIControl, public PropertyListDelegate
{
    
public:
    NodePropertyControl(const Rect & rect, bool showMatrix);
    virtual ~NodePropertyControl();
    
    virtual void WillAppear();

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void WriteTo(SceneNode *sceneNode);
    virtual void InitProperties();
    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey);
    virtual void OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4);

    void SetDelegate(NodePropertyDelegate *delegate);

protected:

    NodePropertyDelegate *nodeDelegate;
    PropertyList *propertyList;
    
    bool showMatrix;
};



#endif // __NODE_PROPERTY_CONTROL_H__