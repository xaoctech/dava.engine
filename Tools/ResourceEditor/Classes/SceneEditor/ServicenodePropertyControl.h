#ifndef __SERVICENODE_PROPERTY_CONTROL_H__
#define __SERVICENODE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class ServicenodePropertyControl: public NodePropertyControl
{
public:

    ServicenodePropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
};



#endif // __SERVICENODE_PROPERTY_CONTROL_H__