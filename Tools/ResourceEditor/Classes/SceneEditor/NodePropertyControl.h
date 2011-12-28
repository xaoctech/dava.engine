#ifndef __NODE_PROPERTY_CONTROL_H__
#define __NODE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

using namespace DAVA;


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

protected:

    
    PropertyList *propertyList;
};



#endif // __NODE_PROPERTY_CONTROL_H__