#ifndef __BOX_PROPERTY_CONTROL_H__
#define __BOX_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class BoxPropertyControl: public NodePropertyControl
{
public:

    BoxPropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void WriteTo(SceneNode *sceneNode);
    virtual void InitProperties();
};



#endif // __BOX_PROPERTY_CONTROL_H__