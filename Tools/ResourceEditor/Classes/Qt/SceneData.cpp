#include "SceneData.h"
#include "SceneGraphModel.h"

#include "../EditorScene.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneValidator.h"

#include "../SceneEditor/SceneEditorScreenMain.h"

#include "QtUtils.h"
#include "GUIActionHandler.h"

#include "LibraryModel.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QAction>

using namespace DAVA;

SceneData::SceneData()
    :   QObject(NULL)
    ,   scene(NULL)
    ,   sceneGraphModel(NULL)
{
    sceneFilePathname = String("");
    sceneGraphModel = new SceneGraphModel();
    sceneGraphModel->SetScene(NULL);
    
    connect(sceneGraphModel, SIGNAL(SceneNodeSelected(DAVA::SceneNode *)), this, SLOT(SceneNodeSelected(DAVA::SceneNode *)));

    libraryModel = new LibraryModel(this);
    
    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
}

SceneData::~SceneData()
{
    SafeRelease(scene);
    
    SafeDelete(libraryModel);
    SafeDelete(sceneGraphModel);
    SafeRelease(cameraController);
}


void SceneData::SetScene(EditorScene *newScene)
{
    ReleaseScene();
    
    scene = SafeRetain(newScene);
    sceneGraphModel->SetScene(scene);
    cameraController->SetScene(scene);
}

void SceneData::RebuildSceneGraph()
{
    sceneGraphModel->Rebuild();
}

EditorScene * SceneData::GetScene()
{
    return scene;
}

void SceneData::AddSceneNode(DAVA::SceneNode *node)
{
    scene->AddNode(node);
    
    RebuildSceneGraph();
}

void SceneData::RemoveSceneNode(DAVA::SceneNode *node)
{
    SceneNode * parent = node->GetParent();
    if (parent)
    {
        scene->ReleaseUserData(node);
        
        sceneGraphModel->SelectNode(NULL);
        scene->SetSelection(NULL);
        
        parent->RemoveNode(node);

        SceneValidator::Instance()->EnumerateSceneTextures();
    }
    
    RebuildSceneGraph();
}

void SceneData::SelectNode(DAVA::SceneNode *node)
{
    sceneGraphModel->SelectNode(node);
}

void SceneData::SceneNodeSelected(SceneNode *node)
{
    if(scene)   scene->SetSelection(node);
    
    cameraController->SetSelection(node);

    //TODO: remove code at full-qt version
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->SelectNodeQt(node);
    }
    //EndOfTODO
    
    
    Camera * cam = dynamic_cast<Camera*>(node);
    if (cam)
    {
        if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT))
        {
            scene->SetClipCamera(cam);
        }
        else
        {
            scene->SetCurrentCamera(cam);
        }
    }
}



DAVA::SceneNode * SceneData::GetSelectedNode()
{
    return sceneGraphModel->GetSelectedNode();
}

void SceneData::LockAtSelectedNode()
{
    if(cameraController)
    {
        cameraController->LockAtSelection();
    }
}

CameraController * SceneData::GetCameraController()
{
    return cameraController;
}

void SceneData::ReleaseScene()
{
    cameraController->SetScene(NULL);
    sceneGraphModel->SetScene(NULL);
    SafeRelease(scene);
}

void SceneData::CreateScene(bool createEditorCameras)
{
    ReleaseScene();

    cameraController->SetSpeed(EditorSettings::Instance()->GetCameraSpeed());
    
    EditorScene *createdScene = new EditorScene();

    // Camera setup
    if(createEditorCameras)
    {
        Camera * cam = new Camera();
        cam->SetName("editor.main-camera");
        cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        
        cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        
        createdScene->AddNode(cam);
        createdScene->AddCamera(cam);
        createdScene->SetCurrentCamera(cam);
        cameraController->SetScene(createdScene);
        
        SafeRelease(cam);
        
        Camera * cam2 = new Camera();
        cam2->SetName("editor.debug-camera");
        cam2->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam2->SetPosition(Vector3(0.0f, 0.0f, 200.0f));
        cam2->SetTarget(Vector3(0.0f, 250.0f, 0.0f));
        
        cam2->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        
        createdScene->AddNode(cam2);
        createdScene->AddCamera(cam2);
        
        SafeRelease(cam2);
    }
    
    SetScene(createdScene);
}

void SceneData::AddScene(const String &scenePathname)
{
    String extension = FileSystem::Instance()->GetExtension(scenePathname);
    DVASSERT((".sc2" == extension) && "Wrong file extension.");
    
    SceneNode * rootNode = scene->GetRootNode(scenePathname)->Clone();
    
    KeyedArchive * customProperties = rootNode->GetCustomProperties();
    customProperties->SetString("editor.referenceToOwner", scenePathname);
    
    rootNode->SetSolid(true);
    scene->AddNode(rootNode);
    
    Camera *currCamera = scene->GetCurrentCamera();
    if(currCamera)
    {
        Vector3 pos = currCamera->GetPosition();
        Vector3 direction  = currCamera->GetDirection();
        
        Vector3 nodePos = pos + 10 * direction;
        nodePos.z = 0;
        
        LandscapeNode * ls = scene->GetLandScape(scene);
        if(ls)
        {
            Vector3 result;
            bool res = ls->PlacePoint(nodePos, result);
            if(res)
            {
                nodePos = result;
            }
        }
        
        Matrix4 mod;
        mod.CreateTranslation(nodePos);
        rootNode->SetLocalTransform(rootNode->GetLocalTransform() * mod);
    }
    SafeRelease(rootNode);

    //TODO: need selection?
//    SelectNode(scene->GetSelection());
    
    RebuildSceneGraph();
    SceneValidator::Instance()->ValidateScene(scene);
    SceneValidator::Instance()->EnumerateSceneTextures();
}

void SceneData::EditScene(const String &scenePathname)
{
    String extension = FileSystem::Instance()->GetExtension(scenePathname);
    DVASSERT((".sc2" == extension) && "Wrong file extension.");

    SceneNode * rootNode = scene->GetRootNode(scenePathname);
    if(rootNode)
    {
        SetScenePathname(scenePathname);
        for (int ci = 0; ci < rootNode->GetChildrenCount(); ++ci)
        {
            //рут нода это сама сцена в данном случае
            scene->AddNode(rootNode->GetChild(ci));
        }
    }

    
    //TODO: need selection?
//    SelectNode(scene->GetSelection());
    
    RebuildSceneGraph();
    SceneValidator::Instance()->ValidateScene(scene);
    SceneValidator::Instance()->EnumerateSceneTextures();
}

void SceneData::SetScenePathname(const String &newPathname)
{
    sceneFilePathname = newPathname;
    if(scene)
    {
        String filename, path;
        FileSystem::Instance()->SplitPath(sceneFilePathname, path, filename);
        scene->SetName(filename);
    }
}

String SceneData::GetScenePathname() const
{
    return sceneFilePathname;
}

void SceneData::Activate(QTreeView *graphview, QTreeView *_libraryView)
{
	libraryView = _libraryView;
		
    sceneGraphModel->Activate(graphview);
    libraryModel->Activate(libraryView);
}

void SceneData::Deactivate()
{
    sceneGraphModel->Deactivate();
    libraryModel->Deactivate();
}

void SceneData::ShowLibraryMenu(const QModelIndex &index, const QPoint &point)
{
    if(!index.isValid())
    {
        return;
    }
    
    QFileInfo fileInfo = libraryModel->fileInfo(index);
    if(fileInfo.isFile())
    {
        String extension = FileSystem::Instance()->GetExtension(QSTRING_TO_DAVASTRING(fileInfo.fileName()));
        if(0 == CompareStrings(String(".sc2"), extension))
        {
            QVariant filePathname(fileInfo.filePath());
            
            QMenu menu;
            QAction *add = menu.addAction(QString("Add"));
            QAction *edit = menu.addAction(QString("Edit"));
            QAction *reload = menu.addAction(QString("Reload"));
            
            add->setData(filePathname);
            edit->setData(filePathname);
            reload->setData(filePathname);
            
            connect(&menu, SIGNAL(triggered(QAction *)), GUIActionHandler::Instance(), SLOT(LibraryMenuTriggered(QAction *)));
            
            menu.exec(point);
        }
        else if(0 == CompareStrings(String(".dae"), extension))
        {
            QVariant filePathname(fileInfo.filePath());

            QMenu menu;
            QAction *convert = menu.addAction(QString("Convert"));
            convert->setData(filePathname);

            connect(&menu, SIGNAL(triggered(QAction *)) , GUIActionHandler::Instance(), SLOT(LibraryMenuTriggered(QAction *)));
            
            menu.exec(point);
        }
    }
}

void SceneData::ReloadRootNode(const DAVA::String &scenePathname)
{
    scene->ReleaseRootNode(scenePathname);
    
    ReloadNode(scene, scenePathname);
    
    scene->SetSelection(0);
    for (int32 i = 0; i < (int32)nodesToAdd.size(); i++)
    {
        scene->ReleaseUserData(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->RemoveNode(nodesToAdd[i].nodeToRemove);
        nodesToAdd[i].parent->AddNode(nodesToAdd[i].nodeToAdd);
        SafeRelease(nodesToAdd[i].nodeToAdd);
    }
    nodesToAdd.clear();
    
    
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->OnReloadRootNodesQt();
    }
    
    RebuildSceneGraph();
}

void SceneData::ReloadNode(SceneNode *node, const String &nodePathname)
{//если в рут ноды сложить такие же рут ноды то на релоаде все накроет пиздой
    KeyedArchive *customProperties = node->GetCustomProperties();
    if (customProperties->GetString("editor.referenceToOwner", "") == nodePathname)
    {
        SceneNode *newNode = scene->GetRootNode(nodePathname)->Clone();
        newNode->SetLocalTransform(node->GetLocalTransform());
        newNode->GetCustomProperties()->SetString("editor.referenceToOwner", nodePathname);
        newNode->SetSolid(true);
        
        SceneNode *parent = node->GetParent();
        AddedNode addN;
        addN.nodeToAdd = newNode;
        addN.nodeToRemove = node;
        addN.parent = parent;
        
        nodesToAdd.push_back(addN);
        return;
    }
    
    int32 csz = node->GetChildrenCount();
    for (int ci = 0; ci < csz; ++ci)
    {
        SceneNode * child = node->GetChild(ci);
        ReloadNode(child, nodePathname);
    }
}

void SceneData::ReloadLibrary()
{
    libraryModel->Reload();
}



void SceneData::BakeNode(DAVA::SceneNode *node)
{
    if(node->GetSolid())
    {
        node->BakeTransforms();
        return;
    }
    
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        BakeNode(node->GetChild(i));
    }
}

void SceneData::RemoveIdentityNodes(DAVA::SceneNode *node)
{
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        SceneNode *removedChild = node->GetChild(i);
        
        if(
           (removedChild->GetFlags() & SceneNode::NODE_LOCAL_MATRIX_IDENTITY)
           &&   (typeid(SceneNode) == typeid(*removedChild))
           &&   (typeid(LodNode) != typeid(*node))
           &&   (removedChild->GetChildrenCount() == 1))
        {
            SceneNode *child = SafeRetain(removedChild->GetChild(0));
            removedChild->RemoveNode(child);
            node->AddNode(child);
            SafeRelease(child);
            
            node->RemoveNode(removedChild);
            
            i = -1;
        }
        else
        {
            RemoveIdentityNodes(removedChild);
        }
    }
}

void SceneData::FindIdentityNodes(DAVA::SceneNode *node)
{
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        SceneNode *child = node->GetChild(i);
        
        if(child->GetSolid())
        {
            RemoveIdentityNodes(child);
        }
        else
        {
            FindIdentityNodes(child);
        }
    }
}


void SceneData::BakeScene()
{
    if(scene)
    {
        SelectNode(NULL);
        
        BakeNode(scene);
        FindIdentityNodes(scene);
        
        RebuildSceneGraph();
    }
}


