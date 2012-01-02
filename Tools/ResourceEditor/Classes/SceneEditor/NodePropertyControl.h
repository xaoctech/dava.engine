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
    NodePropertyControl(const Rect & rect);
    virtual ~NodePropertyControl();
    
    virtual void WillAppear();

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
    
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnItemIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex);

    void SetDelegate(NodePropertyDelegate *delegate);

protected:

    NodePropertyDelegate *nodeDelegate;
    PropertyList *propertyList;
};



#endif // __NODE_PROPERTY_CONTROL_H__