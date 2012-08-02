#ifndef __SCENE_DATA_MANAGER_H__
#define __SCENE_DATA_MANAGER_H__

#include "DAVAEngine.h"

class QTreeView;

class SceneData;
class SceneDataManager: public DAVA::Singleton<SceneDataManager>
{
public:
    SceneDataManager();
    virtual ~SceneDataManager();

    void ActivateScene(DAVA::Scene *scene);
    SceneData *GetActiveScene();
    
    void RegisterNewScene(DAVA::Scene *scene);
    void ReleaseScene(DAVA::Scene *scene);
    DAVA::int32 ScenesCount();

    void SetSceneGraphView(QTreeView *view);
    
protected:

    SceneData * FindDataForScene(DAVA::Scene *scene);
    
protected:
    
    SceneData *currentScene;
    DAVA::List<SceneData *>scenes;
    
    QTreeView *sceneGraphView;
//    QTreeView *dataGraphView;
//    QTreeView *entityGraphView;
    
};

#endif // __SCENE_DATA_MANAGER_H__
