#ifndef __SCENE_DATA_MANAGER_H__
#define __SCENE_DATA_MANAGER_H__

#include "DAVAEngine.h"
#include "EditorScene.h"
#include "Scene/SceneData.h"

#include <QItemSelection>
class QTreeView;

class LibraryModel;
class SceneDataManager: public QObject, public DAVA::Singleton<SceneDataManager>
{
	Q_OBJECT

public:
    SceneDataManager();
    virtual ~SceneDataManager();

    void SetActiveScene(EditorScene *scene);
    SceneData *GetActiveScene();
    SceneData *GetLevelScene();

    EditorScene * RegisterNewScene();
    void ReleaseScene(EditorScene *scene);
    DAVA::int32 ScenesCount();
    SceneData *GetScene(DAVA::int32 index);

    void SetSceneGraphView(QTreeView *view);
    void SetLibraryView(QTreeView *view);
    void SetLibraryModel(LibraryModel *model);
    
	void CompressNotCompressedTextures();
	void ReloadTextures(int32 asFile);
	DAVA::Texture * ReloadTexture(const DAVA::String &descriptorPathname, DAVA::Texture *prevTexture, int32 asFile);

	void EnumerateTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures);


signals:
	void SceneActivated(SceneData *scene);
	void SceneChanged(SceneData *scene);
	void SceneReleased(SceneData *scene);
	void SceneNodeSelected(SceneData *scene, SceneNode *node);

protected slots:
	void InSceneData_SceneChanged(EditorScene *scene);
	void InSceneData_SceneNodeSelected(DAVA::SceneNode *node);

protected:

    SceneData * FindDataForScene(EditorScene *scene);
    

	void RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture);
	void CompressTextures(const List<Texture *> texturesForCompression, ImageFileFormat fileFormat);


protected:
    
    SceneData *currentScene;
    DAVA::List<SceneData *>scenes;
    
    QTreeView *sceneGraphView;
    QTreeView *libraryView;
    LibraryModel *libraryModel;
    
};

#endif // __SCENE_DATA_MANAGER_H__
