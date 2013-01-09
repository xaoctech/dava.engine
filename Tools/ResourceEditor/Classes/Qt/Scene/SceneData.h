#ifndef __SCENE_DATA_H__
#define __SCENE_DATA_H__

#include "DAVAEngine.h"
#include "../SceneEditor/CameraController.h"
#include "../../ParticlesEditorQT/Helpers/ParticlesEditorSceneDataHelper.h"
#include <QObject>

class EditorScene;
class LandscapesController;
class EditorLandscapeNode;

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
    
    void AddScene(const DAVA::String &scenePathname);
    void EditScene(const DAVA::String &scenePathname);
	void AddReferenceScene(const DAVA::String &scenePathname);
    
    void SetScenePathname(const DAVA::String &newPathname);
    DAVA::String GetScenePathname() const;


    void ReloadRootNode(const DAVA::String &scenePathname);

    void BakeScene();
    
    void ToggleNotPassableLandscape();
    
    bool CanSaveScene();
    
    LandscapesController *GetLandscapesController();
    
	void ResetLandsacpeSelection();

	void RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture);

signals:
	void SceneChanged(EditorScene *scene);
	void SceneNodeSelected(DAVA::SceneNode *node);
	
	// Signals are specific for Scene Graph Model.
	void SceneGraphModelNeedsRebuild();
	void SceneGraphModelNeedSetScene(EditorScene* scene);
	void SceneGraphModelNeedsSelectNode(DAVA::SceneNode* node);

protected:
    
    void BakeNode(DAVA::SceneNode *node);
    void FindIdentityNodes(DAVA::SceneNode *node);
    void RemoveIdentityNodes(DAVA::SceneNode *node);
    
    void ReloadNode(DAVA::SceneNode *node, const DAVA::String &nodePathname);

    void ReleaseScene();

protected slots:
    void SceneNodeSelectedInGraph(DAVA::SceneNode *node);

protected:
    EditorScene *scene;

	// Node currently selected.
	DAVA::SceneNode* selectedNode;
	
	// Controllers related to SceneData.
    DAVA::WASDCameraController *cameraController;
    LandscapesController *landscapesController;
	
    DAVA::String sceneFilePathname;
    
    //reload root nodes
    struct AddedNode
    {
        DAVA::SceneNode *nodeToAdd;
        DAVA::SceneNode *nodeToRemove;
        DAVA::SceneNode *parent;
    };
    DAVA::Vector<AddedNode> nodesToAdd;

	// Particles Editor Scene Data Helper.
	DAVA::ParticlesEditorSceneDataHelper particlesEditorSceneDataHelper;
};

#endif // __SCENE_DATA_H__
