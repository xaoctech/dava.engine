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
    
	virtual void Update(float32 timeElapsed);
    
    void SetWorkingScene(Scene *scene);

protected:

    void SetIntInfoValue(const String &key, int32 newValue);
    void SetFloatInfoValue(const String &key, float32 newValue);
    
    void UpdateInfo();
    
    float32 updateTimer;
    
    PropertyList *sceneInfo;
    Scene *workingScene;
};



#endif // __SCENE_INFO_CONTROL_H__