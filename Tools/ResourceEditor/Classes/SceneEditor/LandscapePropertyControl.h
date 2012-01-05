#ifndef __LANDSCAPE_PROPERTY_CONTROL_H__
#define __LANDSCAPE_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
#include "NodePropertyControl.h"

using namespace DAVA;


class LandscapePropertyControl: public NodePropertyControl
{
public:

    LandscapePropertyControl(const Rect & rect, bool showMatrix);

    virtual void ReadFromNode(SceneNode *sceneNode);
    virtual void ReadToNode(SceneNode *sceneNode);
    virtual void SetDefaultValues();
    virtual void InitProperties();
    
    void SetProjectPath(const String &path);
    
private:
        
    bool IsValidPath(const String &path);
    
    String projectPath;
    Vector<String> renderingModes;
    
};

#endif // __LANDSCAPE_PROPERTY_CONTROL_H__
