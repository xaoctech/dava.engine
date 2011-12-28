#ifndef __SPHERE_PROPERTY_CONTROL_H__
#define __SPHERE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class SpherePropertyControl: public NodePropertyControl
{
public:

    SpherePropertyControl(const Rect & rect);

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
};



#endif // __SPHERE_PROPERTY_CONTROL_H__