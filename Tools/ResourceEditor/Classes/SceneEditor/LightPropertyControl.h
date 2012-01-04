#ifndef __LIGHT_PROPERTY_CONTROL_H__
#define __LIGHT_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class LightPropertyControl: public NodePropertyControl
{
public:

    LightPropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
    
protected:
    
    Vector<String> types;
};



#endif // __LIGHT_PROPERTY_CONTROL_H__