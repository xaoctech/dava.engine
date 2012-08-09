#ifndef __SCENE_DATA_H__
#define __SCENE_DATA_H__

#include "DAVAEngine.h"
#include "../SceneEditor/CameraController.h"

#include <QObject>

class EditorScene;
class SceneGraphModel;
class QTreeView;
class SceneData: public QObject
{
    friend class SceneDataManager;
    
    Q_OBJECT
    
public:
    SceneData();
    virtual ~SceneData();

    void RebuildSceneGraph();
    
    void SetScene(EditorScene *newScene);
    EditorScene * GetScene();
    
    void AddSceneNode(DAVA::SceneNode *node);
    void RemoveSceneNode(DAVA::SceneNode *node);

    void SelectNode(DAVA::SceneNode *node);
    DAVA::SceneNode * GetSelectedNode();
    
    void LockAtSelectedNode();
    
    DAVA::CameraController *GetCameraController();
    
    void CreateScene(bool createEditorCameras);
    
    void OpenScene(const DAVA::String &scenePathname);
    void EditScene(const DAVA::String &scenePathname);
    
    void SetScenePathname(const DAVA::String &newPathname);
    DAVA::String GetScenePathname() const;

    void Activate(QTreeView *view);
    void Deactivate();

    
protected:
    
    void ReleaseScene();

protected slots:
    
    void SceneNodeSelected(DAVA::SceneNode *node);
    
    
protected:

    EditorScene *scene;

    DAVA::WASDCameraController *cameraController;
    
    DAVA::String sceneFilePathname;
    
    
    SceneGraphModel *sceneGraphModel;
    //DATA
    //ENTITY
    //PROPERTY
    //LIBRARY
};

#endif // __SCENE_DATA_H__
