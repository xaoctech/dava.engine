#ifndef __CAMERA_PROPERTY_CONTROL_H__
#define __CAMERA_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class CameraPropertyControl: public NodePropertyControl
{
public:

    CameraPropertyControl(const Rect & rect);

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
};



#endif // __CAMERA_PROPERTY_CONTROL_H__