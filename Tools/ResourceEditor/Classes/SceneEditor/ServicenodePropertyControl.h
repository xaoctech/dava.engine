#ifndef __SERVICENODE_PROPERTY_CONTROL_H__
#define __SERVICENODE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class ServicenodePropertyControl: public NodePropertyControl
{
public:

    ServicenodePropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void WriteTo(SceneNode *sceneNode);
    virtual void InitProperties();
};



#endif // __SERVICENODE_PROPERTY_CONTROL_H__