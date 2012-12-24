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

    // void SetSceneGraphView(QTreeView *view);
	// <--

public:
	DAVA::SceneNode*	SceneGetSelectedNode(SceneData *scene);
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

signals:
	void SceneCreated(SceneData *scene);
	void SceneActivated(SceneData *scene);
	void SceneDeactivated(SceneData *scene);
	void SceneChanged(SceneData *scene);
	void SceneReleased(SceneData *scene);
	void SceneNodeSelected(SceneData *scene, DAVA::SceneNode *node);
	
	// Signals needed for Scene Graph Tree View.
	void SceneGraphNeedRebuild();
	void SceneGraphNeedSetScene(SceneData *sceneData, EditorScene *scene);
	void SceneGraphNeedSelectNode(SceneData *sceneData, DAVA::SceneNode* node);

protected slots:
	void InSceneData_SceneChanged(EditorScene *scene);
	void InSceneData_SceneNodeSelected(DAVA::SceneNode *node);
	
	void InSceneData_SceneGraphModelNeedsRebuild();
	void InSceneData_SceneGraphModelNeedSetScene(EditorScene* scene);
	void InSceneData_SceneGraphModelNeedsSelectNode(DAVA::SceneNode* node);

protected:

    SceneData * FindDataForScene(EditorScene *scene);
    
	static void CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, DAVA::LandscapeNode *forNode);
	static void CollectMeshTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, DAVA::MeshInstanceNode *forNode);
	static void CollectTexture(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, const DAVA::String &name, DAVA::Texture *tex);

	void RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture);
	void CompressTextures(const List<Texture *> texturesForCompression, ImageFileFormat fileFormat);


protected:
    SceneData *currentScene;
    DAVA::List<SceneData *>scenes;
};

#endif // __SCENE_DATA_MANAGER_H__
