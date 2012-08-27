#ifndef __SCENE_DATA_MANAGER_H__
#define __SCENE_DATA_MANAGER_H__

#include "DAVAEngine.h"

#include <QItemSelection>
class QTreeView;

class SceneData;
class EditorScene;
class SceneDataManager: public DAVA::Singleton<SceneDataManager>
{
public:
    SceneDataManager();
    virtual ~SceneDataManager();

    void ActivateScene(EditorScene *scene);
    SceneData *GetActiveScene();
    SceneData *GetLevelScene();

    EditorScene * RegisterNewScene();
    void ReleaseScene(EditorScene *scene);
    DAVA::int32 ScenesCount();

    void SetSceneGraphView(QTreeView *view);
    void SetLibraryView(QTreeView *view);
    
protected:

    SceneData * FindDataForScene(EditorScene *scene);
    
protected:
    
    SceneData *currentScene;
    DAVA::List<SceneData *>scenes;
    
    QTreeView *sceneGraphView;
    QTreeView *libraryView;
    
};

#endif // __SCENE_DATA_MANAGER_H__
