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
    
    void SetScenePathname(const DAVA::String &newPathname);
    DAVA::String GetScenePathname() const;

    void Activate(QTreeView *graphview, QTreeView *libraryView);
    void Deactivate();

    void ShowLibraryMenu(const QModelIndex &index, const QPoint &point);
    
    void ReloadRootNode(const DAVA::String &scenePathname);

	void ReloadLibrary();
    
    void BakeScene();
    
protected:
    
    void BakeNode(DAVA::SceneNode *node);
    void FindIdentityNodes(DAVA::SceneNode *node);
    void RemoveIdentityNodes(DAVA::SceneNode *node);

    
    void ReloadNode(DAVA::SceneNode *node, const DAVA::String &nodePathname);

    void ReleaseScene();
    void Execute(Command *command);

protected slots:
    
    void SceneNodeSelected(DAVA::SceneNode *node);
    
    //library
    void LibraryContextMenuRequested(const QPoint &point);
    void LibraryMenuTriggered(QAction *fileAction);
    void FileSelected(const QString &filePathname, bool isFile);

    
protected:

    EditorScene *scene;

    DAVA::WASDCameraController *cameraController;
    
    DAVA::String sceneFilePathname;
    
    
    SceneGraphModel *sceneGraphModel;
    //DATA
    //ENTITY
    //PROPERTY
    //LIBRARY
    LibraryModel *libraryModel;
    
    //reload root nodes
    struct AddedNode
    {
        DAVA::SceneNode *nodeToAdd;
        DAVA::SceneNode *nodeToRemove;
        DAVA::SceneNode *parent;
    };
    DAVA::Vector<AddedNode> nodesToAdd;

	QTreeView *libraryView;
};

#endif // __SCENE_DATA_H__
