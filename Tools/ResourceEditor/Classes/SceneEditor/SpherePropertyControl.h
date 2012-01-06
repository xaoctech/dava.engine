#ifndef __SPHERE_PROPERTY_CONTROL_H__
#define __SPHERE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class SpherePropertyControl: public NodePropertyControl
{
public:

    SpherePropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void WriteTo(SceneNode *sceneNode);
    virtual void InitProperties();
};



#endif // __SPHERE_PROPERTY_CONTROL_H__