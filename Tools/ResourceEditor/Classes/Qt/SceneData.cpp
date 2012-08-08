#include "SceneData.h"
#include "SceneGraphModel.h"

#include "../EditorScene.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneValidator.h"

#include "../SceneEditor/SceneEditorScreenMain.h"

#include <QTreeView>

using namespace DAVA;

SceneData::SceneData()
    :   scene(NULL)
    ,   sceneGraphModel(NULL)
{
    sceneFilePathname = String("");
    sceneGraphModel = new SceneGraphModel();
    sceneGraphModel->SetScene(NULL);
    
    connect(sceneGraphModel, SIGNAL(SceneNodeSelected(DAVA::SceneNode *)), this, SLOT(SceneNodeSelected(DAVA::SceneNode *)));

    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
}

SceneData::~SceneData()
{
    ReleaseScene();
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

void SceneData::OpenScene(const String &scenePathname)
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

void SceneData::Activate(QTreeView *view)
{
    sceneGraphModel->Activate(view);
}

void SceneData::Deactivate()
{
    sceneGraphModel->Deactivate();
}
