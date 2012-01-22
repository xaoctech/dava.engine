#ifndef __SCENE_INFO_CONTROL_H__
#define __SCENE_INFO_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class PropertyList;
class SceneInfoControl : public UIControl
{
public:
    SceneInfoControl(const Rect &rect);
    virtual ~SceneInfoControl();
    
	virtual void WillAppear();
    void SetWorkingScene(Scene *scene);

protected:

    void SetIntInfoValue(const String &key, int32 newValue);
    void SetFloatInfoValue(const String &key, float32 newValue);
    
    void UpdateInfo(BaseObject * owner, void * userData, void * callerData);
    
    PropertyList *sceneInfo;
    Scene *workingScene;
};



#endif // __SCENE_INFO_CONTROL_H__