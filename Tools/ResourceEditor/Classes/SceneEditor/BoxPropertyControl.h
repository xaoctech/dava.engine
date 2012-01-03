#ifndef __BOX_PROPERTY_CONTROL_H__
#define __BOX_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class BoxPropertyControl: public NodePropertyControl
{
public:

    BoxPropertyControl(const Rect & rect);

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
};



#endif // __BOX_PROPERTY_CONTROL_H__