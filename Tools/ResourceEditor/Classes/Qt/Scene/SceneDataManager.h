#ifndef __SCENE_DATA_MANAGER_H__
#define __SCENE_DATA_MANAGER_H__

#include "DAVAEngine.h"
#include "EditorScene.h"
#include "Scene/SceneData.h"

class SceneDataManager: public QObject, public DAVA::Singleton<SceneDataManager>
{
	Q_OBJECT

public:
    SceneDataManager();
    virtual ~SceneDataManager();

	// TODO:
	// this part need refactor -->
    void SetActiveScene(EditorScene *scene);

	EditorScene * RegisterNewScene();
    void ReleaseScene(EditorScene *scene);

 	// <--

public:
	// Create the new scene.
	SceneData* CreateNewScene();

	// Add the new scene.
	void AddScene(const DAVA::String &scenePathname);

	// Edit the existing level scene.
	void EditLevelScene(const String &scenePathname);

	// Edit the active scene.
	void EditActiveScene(const String &scenePathname);

	// Add the reference scene.
	void AddReferenceScene(const String &scenePathname);

	// Reload the scene.
	void ReloadScene(const String &scenePathname);

	DAVA::SceneNode*	SceneGetSelectedNode(SceneData *scene);
	DAVA::SceneNode*	SceneGetRootNode(SceneData *scene);
	SceneData*			SceneGetActive();
	SceneData*			SceneGetLevel();
	SceneData*			SceneGet(DAVA::int32 index);
	DAVA::int32			SceneCount();
	void				SceneShowPreview(const DAVA::String &path);
	void				SceneHidePreview();
    
	void				TextureCompressAllNotCompressed();
	void				TextureReloadAll(DAVA::ImageFileFormat asFile);
	DAVA::Texture*		TextureReload(const TextureDescriptor *descriptor, DAVA::Texture *prevTexture, DAVA::ImageFileFormat asFile);

	static void EnumerateTextures(DAVA::SceneNode *forNode, DAVA::Map<DAVA::String, DAVA::Texture *> &textures);
	static void EnumerateMaterials(DAVA::SceneNode *forNode, Vector<Material *> &materials);

	// These methods are called by Scene Graph Tree View.
	void SceneNodeSelectedInSceneGraph(SceneNode* node);

	// Refresh the information regarding the particular Particles Editor nods.
	void RefreshParticlesLayer(DAVA::ParticleLayer* layer);

signals:
	void SceneCreated(SceneData *scene);
	void SceneActivated(SceneData *scene);
	void SceneDeactivated(SceneData *scene);
	void SceneChanged(SceneData *scene);
	void SceneReleased(SceneData *scene);
	void SceneNodeSelected(SceneData *scene, DAVA::SceneNode *node);
	
	// Signals needed for Scene Graph Tree View.
	void SceneGraphNeedRebuildNode(DAVA::SceneNode* node);
	void SceneGraphNeedRebuild();
	
	// Signals related to Particles Editor.
	void SceneGraphNeedRefreshLayer(DAVA::ParticleLayer* layer);

	void SceneGraphNeedSetScene(SceneData *sceneData, EditorScene *scene);
	void SceneGraphNeedSelectNode(SceneData *sceneData, DAVA::SceneNode* node);

protected slots:
	void InSceneData_SceneChanged(EditorScene *scene);
	void InSceneData_SceneNodeSelected(DAVA::SceneNode *node);

	// Rebuild the Scene Graph for particular node and for the whole graph.
	void InSceneData_SceneGraphModelNeedsRebuildNode(DAVA::SceneNode *node);
	void InSceneData_SceneGraphModelNeedsRebuild();
	
	void InSceneData_SceneGraphModelNeedSetScene(EditorScene* scene);
	void InSceneData_SceneGraphModelNeedsSelectNode(DAVA::SceneNode* node);

protected:

    SceneData * FindDataForScene(EditorScene *scene);
    
	static void CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, DAVA::Landscape *forNode);
	static void CollectTexture(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, const DAVA::String &name, DAVA::Texture *tex);

	void RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture);
	void CompressTextures(const List<Texture *> texturesForCompression, ImageFileFormat fileFormat);

	// Edit Scene implementation for any kind of scenes.
	void EditScene(SceneData* sceneData, const String &scenePathname);

	// Reload the scene node in a recursive way.
	void ReloadNode(EditorScene* scene, SceneNode *node, const String &nodePathname);

	// Update the Particle Editor sprites.
	void UpdateParticleSprites();

protected:
    SceneData *currentScene;
    DAVA::List<SceneData *>scenes;
	
	// This structure is needed to reload scene.
    struct AddedNode
    {
        DAVA::SceneNode *nodeToAdd;
        DAVA::SceneNode *nodeToRemove;
        DAVA::SceneNode *parent;
    };
	
    DAVA::Vector<AddedNode> nodesToAdd;
};

#endif // __SCENE_DATA_MANAGER_H__
