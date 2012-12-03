#ifndef __SCENE_DATA_H__
#define __SCENE_DATA_H__

#include "DAVAEngine.h"
#include "../SceneEditor/CameraController.h"

#include <QObject>
#include <QModelIndex>
#include <QPoint>

class QFileSystemModel;
class QTreeView;
class EditorScene;
class SceneGraphModel;
class LibraryModel;
class Command;
class QAction;
class QMenu;
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

    void Activate(QTreeView *graphview, QTreeView *libraryView, LibraryModel *libModel);
    void Deactivate();

    void ReloadRootNode(const DAVA::String &scenePathname);

	void ReloadLibrary();
    
    void BakeScene();
    
    void ToggleNotPassableLandscape();
    
    bool CanSaveScene();
    
    LandscapesController *GetLandscapesController();
    
    void OpenLibraryForFile(const DAVA::String &filePathname);
    
	void ResetLandsacpeSelection();

	void EnumerateTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures);
	void RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture);


signals:
	void SceneChanged(EditorScene *scene);
	void SceneNodeSelected(DAVA::SceneNode *node);

protected:
    
    void BakeNode(DAVA::SceneNode *node);
    void FindIdentityNodes(DAVA::SceneNode *node);
    void RemoveIdentityNodes(DAVA::SceneNode *node);
    
    void ReloadNode(DAVA::SceneNode *node, const DAVA::String &nodePathname);

    void ReleaseScene();
    void Execute(Command *command);
    
    void ShowLibraryMenu(const QModelIndex &index, const QPoint &point);
    void ShowSceneGraphMenu(const QModelIndex &index, const QPoint &point);

    void ProcessContextMenuAction(QAction *action);

	void CollectTexture(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, const DAVA::String &name, DAVA::Texture *tex);

protected slots:
    
    void SceneNodeSelectedInGraph(DAVA::SceneNode *node);
    
    //library
    void LibraryContextMenuRequested(const QPoint &point);
    void LibraryMenuTriggered(QAction *action);
    void FileSelected(const QString &filePathname, bool isFile);

    //Scene Graph
    void SceneGraphContextMenuRequested(const QPoint &point);
    void SceneGraphMenuTriggered(QAction *action);
    
protected:

    void AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command);
    
    
    
    
    EditorScene *scene;

    DAVA::WASDCameraController *cameraController;
    
    DAVA::String sceneFilePathname;
    
    
    SceneGraphModel *sceneGraphModel;
    //DATA
    //ENTITY
    //PROPERTY
    LibraryModel *libraryModel;
    
    //reload root nodes
    struct AddedNode
    {
        DAVA::SceneNode *nodeToAdd;
        DAVA::SceneNode *nodeToRemove;
        DAVA::SceneNode *parent;
    };
    DAVA::Vector<AddedNode> nodesToAdd;

	QTreeView *sceneGraphView;
	QTreeView *libraryView;
    
    LandscapesController *landscapesController;
    
    bool skipLibraryPreview;

};

#endif // __SCENE_DATA_H__
