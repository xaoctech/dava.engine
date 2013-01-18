#include "Scene/SceneData.h"
#include "DockSceneGraph/SceneGraphModel.h"

#include "../EditorScene.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneValidator.h"

#include "../SceneEditor/SceneEditorScreenMain.h"

#include "../Commands/SceneGraphCommands.h"
#include "../Commands/LibraryCommands.h"
#include "../Commands/CommandsManager.h"

#include "../LandscapeEditor/LandscapesController.h"

#include "Scene3D/Components/CameraComponent.h"

#include "Main/QtMainWindowHandler.h"


#include "Main/QtUtils.h"
#include "Main/PointerHolder.h"

#include "DockLibrary//LibraryModel.h"

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
	,	selectedNode(NULL)
{
    landscapesController = new LandscapesController();
    sceneFilePathname = String("");
    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
}

SceneData::~SceneData()
{
    SafeRelease(scene);
    SafeRelease(landscapesController);
    SafeRelease(cameraController);
}

void SceneData::RebuildSceneGraphNode(DAVA::SceneNode* node)
{
	emit SceneGraphModelNeedsRebuildNode(node);
}

void SceneData::RebuildSceneGraph()
{
	emit SceneGraphModelNeedsRebuild();
}

EditorScene * SceneData::GetScene()
{
    return scene;
}

void SceneData::AddSceneNode(DAVA::SceneNode *node)
{
    // Firstly ask Particle Editor to add this node.
    if (particlesEditorSceneDataHelper.AddSceneNode(node) == false)
    {
        scene->AddNode(node);
    }
    
    LandscapeNode *landscape = dynamic_cast<LandscapeNode *>(node);
    if(landscape)
    {
        landscapesController->SaveLandscape(landscape);
    }
    
    RebuildSceneGraph();
}

void SceneData::RemoveSceneNode(DAVA::SceneNode *node)
{
    SceneNode * parent = node->GetParent();
    if (parent)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode *>(node);
        if(landscape)
        {
            landscapesController->SaveLandscape(NULL);
        }

		particlesEditorSceneDataHelper.RemoveSceneNode(node);
        scene->ReleaseUserData(node);
		SelectNode(NULL);
        scene->SetSelection(NULL);
        
        parent->RemoveNode(node);

        SceneValidator::Instance()->EnumerateSceneTextures();
    }
    
    RebuildSceneGraph();
}

void SceneData::SelectNode(DAVA::SceneNode *node)
{
	this->selectedNode = node;
	emit SceneGraphModelNeedsSelectNode(node);
}

void SceneData::SceneNodeSelectedInGraph(SceneNode *node)
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
        if (IsKeyModificatorPressed(DVKEY_ALT))
        {
            scene->SetClipCamera(cam);
        }
        else
        {
            scene->SetCurrentCamera(cam);
        }
    }

	this->selectedNode = node;
	emit SceneNodeSelected(node);
}

DAVA::SceneNode * SceneData::GetSelectedNode()
{
    return this->selectedNode;
}

void SceneData::LockAtSelectedNode()
{
    if(cameraController)
    {
        cameraController->LookAtSelection();
    }
}

CameraController * SceneData::GetCameraController()
{
    return cameraController;
}

void SceneData::SetScene(EditorScene *newScene)
{
    ReleaseScene();
    
    scene = SafeRetain(newScene);
	emit SceneGraphModelNeedSetScene(scene);
    cameraController->SetScene(scene);
    landscapesController->SetScene(scene);
}

void SceneData::ReleaseScene()
{
    cameraController->SetScene(NULL);
	emit SceneGraphModelNeedSetScene(NULL);
    landscapesController->SetScene(NULL);
    
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
        cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        
        cam->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        
        ScopedPtr<SceneNode> node(new SceneNode());
        node->SetName("editor.main-camera");
        node->AddComponent(new CameraComponent(cam));
        createdScene->AddNode(node);
        createdScene->AddCamera(cam);
        createdScene->SetCurrentCamera(cam);
        
        SafeRelease(cam);
        
        Camera * cam2 = new Camera();
        cam2->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam2->SetPosition(Vector3(0.0f, 0.0f, 200.0f));
        cam2->SetTarget(Vector3(0.0f, 250.0f, 0.0f));
        
        cam2->Setup(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        
        ScopedPtr<SceneNode> node2(new SceneNode());
        node2->SetName("editor.debug-camera");
        node2->AddComponent(new CameraComponent(cam2));
        createdScene->AddNode(node2);
        createdScene->AddCamera(cam2);
        
        SafeRelease(cam2);
    }
    
    SetScene(createdScene);
    SafeRelease(createdScene);
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
            //SceneNode *child = SafeRetain(removedChild->GetChild(0));
            //removedChild->RemoveNode(child);
            node->AddNode(removedChild->GetChild(0));
            //SafeRelease(child);
            
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

void SceneData::ToggleNotPassableLandscape()
{
    UIScreen *screen = UIScreenManager::Instance()->GetScreen();
    if (IsPointerToExactClass<SceneEditorScreenMain>(screen))
    {
        if(((SceneEditorScreenMain *)screen)->TileMaskEditorEnabled())
        {
            return;
        }

    }
    
    landscapesController->ToggleNotPassableLandscape();
    RebuildSceneGraph();
}


bool SceneData::CanSaveScene()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(!screen->SaveIsAvailable())
    {
        return false;
    }
    
    if(landscapesController->EditorLandscapeIsActive())
    {
        return false;
    }
    
    
    return true;
}


LandscapesController * SceneData::GetLandscapesController()
{
    return landscapesController;
}

void SceneData::SetLandscapesControllerScene(EditorScene* scene)
{
	if (this->landscapesController)
	{
		this->landscapesController->SetScene(scene);
	}
}

void SceneData::ResetLandsacpeSelection()
{
	LandscapeNode *selectedNode = dynamic_cast<LandscapeNode *>	(GetSelectedNode());
	if(selectedNode)
	{
		SelectNode(NULL);
	}
}


void SceneData::RestoreTexture(const DAVA::String &descriptorPathname, DAVA::Texture *texture)
{
	//materials
	Vector<Material*> materials;
	scene->GetDataNodes(materials);
	for(int32 m = 0; m < (int32)materials.size(); ++m)
	{
		for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
		{
			if(materials[m]->GetTextureName((Material::eTextureLevel)t) == descriptorPathname)
			{
				materials[m]->SetTexture((Material::eTextureLevel)t, texture);
			}
		}
	}

	//landscapes
	Vector<LandscapeNode *> landscapes;
	scene->GetChildNodes(landscapes);
	for(int32 l = 0; l < (int32)landscapes.size(); ++l)
	{
		for(int32 t = 0; t < LandscapeNode::TEXTURE_COUNT; t++)
		{
			if(landscapes[l]->GetTextureName((LandscapeNode::eTextureLevel)t) == descriptorPathname)
			{
				landscapes[l]->SetTexture((LandscapeNode::eTextureLevel)t, texture);
			}
		}
	}

	//lightmaps
	Vector<MeshInstanceNode *> meshInstances;
	scene->GetChildNodes(meshInstances);
	for(int32 m = 0; m < (int32)meshInstances.size(); ++m)
	{
		for (int32 li = 0; li < meshInstances[m]->GetLightmapCount(); ++li)
		{
			MeshInstanceNode::LightmapData * ld = meshInstances[m]->GetLightmapDataForIndex(li);
			if (ld)
			{
				SafeRelease(ld->lightmap);
				ld->lightmap = SafeRetain(texture);
			}
		}
	}
}

void SceneData::EmitSceneChanged()
{
	emit SceneChanged(this->scene);
}
