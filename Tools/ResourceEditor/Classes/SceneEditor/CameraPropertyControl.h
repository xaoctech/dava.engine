#ifndef __CAMERA_PROPERTY_CONTROL_H__
#define __CAMERA_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class CameraPropertyControl: public NodePropertyControl
{
public:

    CameraPropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFrom(SceneNode *sceneNode);
    virtual void WriteTo(SceneNode *sceneNode);
    virtual void InitProperties();
};



#endif // __CAMERA_PROPERTY_CONTROL_H__