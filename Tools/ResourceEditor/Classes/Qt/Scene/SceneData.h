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

	// Rebuild the scene graph for particular node.
	void RebuildSceneGraphNode(DAVA::SceneNode* node);
	
	// Rebuild the whole scene graph.
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

    void SetScenePathname(const DAVA::String &newPathname);
    DAVA::String GetScenePathname() const;

    void ToggleNotPassableLandscape();
    
    bool CanSaveScene();
    
    LandscapesController *GetLandscapesController();
	void SetLandscapesControllerScene(EditorScene* scene);
    
	void ResetLandsacpeSelection();

	void RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture);

	// Emit the SceneChanged singal.
	void EmitSceneChanged();

	void GetAllParticleEffects(DAVA::List<DAVA::SceneNode*> & particleEffects);
	void GetAllSwitchComponents(DAVA::List<DAVA::SceneNode*> & particleEffects);
signals:
	void SceneChanged(EditorScene *scene);
	void SceneNodeSelected(DAVA::SceneNode *node);
	
	// Signals are specific for Scene Graph Model.
	void SceneGraphModelNeedsRebuildNode(DAVA::SceneNode *node);
	void SceneGraphModelNeedsRebuild();
	
	void SceneGraphModelNeedSetScene(EditorScene* scene);
	void SceneGraphModelNeedsSelectNode(DAVA::SceneNode* node);

protected:
    
    void BakeNode(DAVA::SceneNode *node);
    void FindIdentityNodes(DAVA::SceneNode *node);
    void RemoveIdentityNodes(DAVA::SceneNode *node);
    
    void ReloadNode(DAVA::SceneNode *node, const DAVA::String &nodePathname);

    void ReleaseScene();

	void FindAllParticleEffectsRecursive(DAVA::SceneNode *node , DAVA::List<DAVA::SceneNode*> & particleEffects);

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
    
	// Particles Editor Scene Data Helper.
	DAVA::ParticlesEditorSceneDataHelper particlesEditorSceneDataHelper;
};

#endif // __SCENE_DATA_H__
