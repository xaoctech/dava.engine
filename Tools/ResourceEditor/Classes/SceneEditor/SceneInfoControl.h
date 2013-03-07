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

    void InvalidateTexturesInfo(int32 count, int32 size);
    void SetRenderStats(const RenderManager::Stats & newRenderStats);
    
    void SetNodesCount(int32 nodesCount);
    
protected:

    void InvalidateRenderStats();
    void SetIntInfoValue(const String &key, int32 newValue);
    void SetFloatInfoValue(const String &key, float32 newValue);
    void SetStringInfoValue(const String &key, const String &newString);
    
    void UpdateInfo(BaseObject * owner, void * userData, void * callerData);
    void RedrawCells();
    
    PropertyList *sceneInfo;
    
    RenderManager::Stats renderStats;
    
    
    Scene * GetWorkingScene() const;
};



#endif // __SCENE_INFO_CONTROL_H__