#include "SceneDataManager.h"

#include "DockSceneGraph/SceneGraphModel.h"

#include "../SceneEditor/SceneValidator.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/PVRConverter.h"

#include "./ParticlesEditorQT/Helpers/ParticlesEditorSpritePackerHelper.h"

using namespace DAVA;

SceneDataManager::SceneDataManager()
    :   currentScene(NULL)
{
}

SceneDataManager::~SceneDataManager()
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SafeDelete(*it);
    }
    scenes.clear();
}

SceneData* SceneDataManager::CreateNewScene()
{
	SceneData *levelScene = SceneGetLevel();
    levelScene->CreateScene(true);
	
	UpdateParticleSprites();
	return levelScene;	
}

void SceneDataManager::AddScene(const String &scenePathname)
{
    String extension = FileSystem::Instance()->GetExtension(scenePathname);
    DVASSERT((".sc2" == extension) && "Wrong file extension.");

	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to add the scene when SceneGetActive() returns NULL!");
		return;
	}
	
	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return;
	}

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
	
	List<LandscapeNode *> landscapes;
	rootNode->GetChildNodes(landscapes);
	
	bool needUpdateLandscapeController = !landscapes.empty();
	
    SafeRelease(rootNode);
	
    //TODO: need save scene automatically?
    bool changesWereMade = SceneValidator::Instance()->ValidateSceneAndShowErrors(scene);
    SceneValidator::Instance()->EnumerateSceneTextures();
	
	if(needUpdateLandscapeController)
	{
		sceneData->SetLandscapesControllerScene(scene);
	}

	UpdateParticleSprites();
	emit SceneGraphNeedRebuild();
}

void SceneDataManager::EditLevelScene(const String &scenePathname)
{
	SceneData* sceneData = SceneGetLevel();
	if (!sceneData)
	{
		DVASSERT(false && "No way to edit the scene when SceneGetLevel() returns NULL!");
		return;
	}
	
	return EditScene(sceneData, scenePathname);
}

void SceneDataManager::EditActiveScene(const String &scenePathname)
{
	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to edit the scene when SceneGetActive() returns NULL!");
		return;
	}
	
	return EditScene(sceneData, scenePathname);
}

void SceneDataManager::EditScene(SceneData* sceneData, const String &scenePathname)
{
	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return;
	}

    String extension = FileSystem::Instance()->GetExtension(scenePathname);
    DVASSERT((".sc2" == extension) && "Wrong file extension.");
	
    SceneNode * rootNode = scene->GetRootNode(scenePathname);
    if(rootNode)
    {
        sceneData->SetScenePathname(scenePathname);
		Vector<SceneNode*> tempV;
		tempV.reserve(rootNode->GetChildrenCount());
		
		for (int32 ci = 0; ci < rootNode->GetChildrenCount(); ++ci)
		{
			tempV.push_back(rootNode->GetChild(ci));
		}
        for (int32 ci = 0; ci < (int32)tempV.size(); ++ci)
        {
            //рут нода это сама сцена в данном случае
            scene->AddNode(tempV[ci]);
        }
    }

    //TODO: need save scene automatically?
    bool changesWereMade = SceneValidator::Instance()->ValidateSceneAndShowErrors(scene);
    SceneValidator::Instance()->EnumerateSceneTextures();

    sceneData->SetLandscapesControllerScene(scene);
	
	scene->Update(0);
	sceneData->EmitSceneChanged();

	UpdateParticleSprites();
    emit SceneGraphNeedRebuild();
}

void SceneDataManager::AddReferenceScene(const String &scenePathname)
{
	String extension = FileSystem::Instance()->GetExtension(scenePathname);
	DVASSERT((".sc2" == extension) && "Wrong file extension.");

	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to add reference scene when SceneGetActive() returns NULL!");
		return;
	}

	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return;
	}

	SceneNode * rootNode = scene->GetRootNode(scenePathname);
	
	DVASSERT(rootNode->GetChildrenCount() == 1);
	ReferenceNode * refNode = new ReferenceNode();
	SceneNode * clone = rootNode->GetChild(0)->Clone();
	refNode->AddNode(clone);
	refNode->SetName(rootNode->GetName());
	SafeRelease(clone);
	
	KeyedArchive * customProperties = refNode->GetCustomProperties();
	customProperties->SetString("reference.path", scenePathname);
	
	refNode->SetSolid(true);
	scene->AddNode(refNode);
	
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
		refNode->SetLocalTransform(refNode->GetLocalTransform() * mod);
	}
	SafeRelease(refNode);
	
	UpdateParticleSprites();
	emit SceneGraphNeedRebuild();
	
    //TODO: need save scene automatically?
    bool changesWereMade = SceneValidator::Instance()->ValidateSceneAndShowErrors(scene);
	SceneValidator::Instance()->EnumerateSceneTextures();
}

void SceneDataManager::ReloadScene(const String &scenePathname)
{
	SceneData* sceneData = SceneGetActive();
	if (!sceneData)
	{
		DVASSERT(false && "No way to add reference scene when SceneGetActive() returns NULL!");
		return;
	}
	
	EditorScene* scene = sceneData->GetScene();
	if (!scene)
	{
		DVASSERT(false && "sceneData->GetScene() returned NULL!");
		return;
	}

	sceneData->SelectNode(NULL);
    scene->ReleaseRootNode(scenePathname);

	nodesToAdd.clear();
    ReloadNode(scene, scene, scenePathname);

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
    
	UpdateParticleSprites();
    emit SceneGraphNeedRebuild();
	sceneData->SetLandscapesControllerScene(scene);
}

void SceneDataManager::ReloadNode(EditorScene* scene, SceneNode *node, const String &nodePathname)
{
	//если в рут ноды сложить такие же рут ноды то на релоаде все накроет пиздой
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
        ReloadNode(scene, child, nodePathname);
    }
}

void SceneDataManager::SetActiveScene(EditorScene *scene)
{
    if(currentScene)
    {
		emit SceneDeactivated(currentScene);
    }
    
    currentScene = FindDataForScene(scene);
    DVASSERT(currentScene && "There is no current scene. Something wrong.");
    currentScene->RebuildSceneGraph();

	emit SceneActivated(currentScene);
}

SceneData * SceneDataManager::FindDataForScene(EditorScene *scene)
{
    SceneData *foundData = NULL;

    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        if((*it)->GetScene() == scene)
        {
            foundData = *it;
            break;
        }
    }
    
    return foundData;
}


SceneData * SceneDataManager::SceneGetActive()
{
	return currentScene;
}

SceneData *SceneDataManager::SceneGetLevel()
{
    if(0 < scenes.size())
    {
        return scenes.front();
    }
    
    return NULL;
}

DAVA::SceneNode* SceneDataManager::SceneGetSelectedNode(SceneData *scene)
{
	DAVA::SceneNode *node = NULL;

	if(NULL != scene)
	{
		node = scene->GetSelectedNode();
	}

	return node;
}

void SceneDataManager::SceneShowPreview(const DAVA::String &path)
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());

	if(screen)
	{
		if(0 == CompareCaseInsensitive(FileSystem::Instance()->GetExtension(path), ".sc2") && FileSystem::Instance()->IsFile(path))
		{
			screen->ShowScenePreview(FileSystem::Instance()->GetCanonicalPath(path));
		}
		else
		{
			SceneHidePreview();
		}
	}
}

void SceneDataManager::SceneHidePreview()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(NULL != screen)
	{
		screen->HideScenePreview();
	}
}

EditorScene * SceneDataManager::RegisterNewScene()
{
    SceneData *data = new SceneData();
    data->CreateScene(true);

    scenes.push_back(data);

	emit SceneCreated(data);

	connect(data, SIGNAL(SceneChanged(EditorScene *)), this, SLOT(InSceneData_SceneChanged(EditorScene *)));
	connect(data, SIGNAL(SceneNodeSelected(DAVA::SceneNode *)), this, SLOT(InSceneData_SceneNodeSelected(DAVA::SceneNode *)));

	connect(data, SIGNAL(SceneGraphModelNeedsRebuildNode(DAVA::SceneNode *)), this, SLOT(InSceneData_SceneGraphModelNeedsRebuildNode(DAVA::SceneNode *)));
	connect(data, SIGNAL(SceneGraphModelNeedsRebuild()), this, SLOT(InSceneData_SceneGraphModelNeedsRebuild()));
	
	connect(data, SIGNAL(SceneGraphModelNeedSetScene(EditorScene *)), this, SLOT(InSceneData_SceneGraphModelNeedSetScene(EditorScene *)));
	connect(data, SIGNAL(SceneGraphModelNeedsSelectNode(DAVA::SceneNode*)), this, SLOT(InSceneData_SceneGraphModelNeedsSelectNode(DAVA::SceneNode*)));

    return data->GetScene();
}

void SceneDataManager::ReleaseScene(EditorScene *scene)
{
    List<SceneData *>::iterator endIt = scenes.end();
    for(List<SceneData *>::iterator it = scenes.begin(); it != endIt; ++it)
    {
        SceneData *sceneData = *it;
        if(sceneData->GetScene() == scene)
        {
			emit SceneReleased(sceneData);

			if(currentScene == sceneData)
            {
                DVASSERT((0 < scenes.size()) && "There is no main level scene.")
                currentScene = *scenes.begin(); // maybe we need to activate next or prev tab?
            }
                
            SafeDelete(sceneData);

            scenes.erase(it);
            break;
        }
    }
}

DAVA::int32 SceneDataManager::SceneCount()
{
    return (int32)scenes.size();
}

SceneData *SceneDataManager::SceneGet(DAVA::int32 index)
{
    DVASSERT((0 <= index) && (index < (int32)scenes.size()));
    
    DAVA::List<SceneData *>::iterator it = scenes.begin();
    std::advance(it, index);
    
    return *it;
}

void SceneDataManager::InSceneData_SceneChanged(EditorScene *scene)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneChanged(sceneData);
}

void SceneDataManager::InSceneData_SceneNodeSelected(SceneNode *node)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneNodeSelected(sceneData, node);
}

void SceneDataManager::InSceneData_SceneGraphModelNeedsRebuildNode(DAVA::SceneNode *node)
{
	// Re-emit the signal from the "inner" Scene Data to all SceneDataManager subscribers.
	emit SceneGraphNeedRebuildNode(node);
}

void SceneDataManager::InSceneData_SceneGraphModelNeedsRebuild()
{
	// Re-emit the signal from the "inner" Scene Data to all SceneDataManager subscribers.
	emit SceneGraphNeedRebuild();
}

void SceneDataManager::InSceneData_SceneGraphModelNeedSetScene(EditorScene *scene)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneGraphNeedSetScene(sceneData, scene);
}

void SceneDataManager::InSceneData_SceneGraphModelNeedsSelectNode(DAVA::SceneNode* node)
{
	SceneData *sceneData = (SceneData *) QObject::sender();
	emit SceneGraphNeedSelectNode(sceneData, node);
}

void SceneDataManager::EnumerateTextures(DAVA::SceneNode *forNode, Map<String, Texture *> &textures)
{
	if(!forNode)  return;

	//materials
	Vector<Material*> materials;
	EnumerateMaterials(forNode, materials);
	for(int32 m = 0; m < (int32)materials.size(); ++m)
	{
		for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
		{
			CollectTexture(textures, materials[m]->GetTextureName((Material::eTextureLevel)t), materials[m]->GetTexture((Material::eTextureLevel)t));
		}
	}

	//landscapes
	Vector<LandscapeNode *> landscapes;
	forNode->GetChildNodes(landscapes);
	LandscapeNode *landscape = dynamic_cast<LandscapeNode *>(forNode);
	if(landscape)
	{
		landscapes.push_back(landscape);
	}

	for(int32 l = 0; l < (int32)landscapes.size(); ++l)
	{
		CollectLandscapeTextures(textures, landscapes[l]);
	}

	//lightmaps
	Vector<MeshInstanceNode *> meshInstances;
	forNode->GetChildNodes(meshInstances);
	MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *>(forNode);
	if(mesh)
	{
		meshInstances.push_back(mesh);
	}
	for(int32 m = 0; m < (int32)meshInstances.size(); ++m)
	{
		CollectMeshTextures(textures, meshInstances[m]);
	}
}

void SceneDataManager::CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, LandscapeNode *forNode)
{
	for(int32 t = 0; t < LandscapeNode::TEXTURE_COUNT; t++)
	{
		CollectTexture(textures, forNode->GetTextureName((LandscapeNode::eTextureLevel)t), forNode->GetTexture((LandscapeNode::eTextureLevel)t));
	}
}

void SceneDataManager::CollectMeshTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, MeshInstanceNode *forNode)
{
	for (int32 li = 0; li < forNode->GetLightmapCount(); ++li)
	{
		MeshInstanceNode::LightmapData * ld = forNode->GetLightmapDataForIndex(li);
		if (ld)
		{
			CollectTexture(textures, ld->lightmapName, ld->lightmap);
		}
	}
}


void SceneDataManager::CollectTexture(Map<String, Texture *> &textures, const String &name, Texture *tex)
{
	if(!name.empty() && SceneValidator::Instance()->IsPathCorrectForProject(name))
	{
		textures[name] = tex;
	}
}


void SceneDataManager::TextureCompressAllNotCompressed()
{
	Map<String, Texture *> textures;
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		EnumerateTextures((*it)->GetScene(), textures);
	}

	List<Texture *>texturesForPVRCompression;
	List<Texture *>texturesForDXTCompression;

	Map<String, Texture *>::const_iterator endItTextures = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endItTextures; ++it)
	{
		if(SceneValidator::Instance()->IsTextureChanged(it->first, PVR_FILE))
		{
			texturesForPVRCompression.push_back(SafeRetain(it->second));
		}
		else if(SceneValidator::Instance()->IsTextureChanged(it->first, DXT_FILE))
		{
			texturesForDXTCompression.push_back(SafeRetain(it->second));
		}
	}

	CompressTextures(texturesForPVRCompression, PVR_FILE);
	CompressTextures(texturesForDXTCompression, DXT_FILE);

	for_each(texturesForPVRCompression.begin(), texturesForPVRCompression.end(),  SafeRelease<Texture>);
	for_each(texturesForDXTCompression.begin(), texturesForDXTCompression.end(),  SafeRelease<Texture>);
}

void SceneDataManager::CompressTextures(const List<DAVA::Texture *> texturesForCompression, DAVA::ImageFileFormat fileFormat)
{
	//TODO: need to run compression at thread

// 	List<Texture *>::const_iterator endIt = texturesForCompression.end();
// 	for(List<Texture *>::const_iterator it = texturesForCompression.begin(); it != endIt; ++it)
// 	{
// 		Texture *texture = *it;
// 		//TODO: compress texture
// 		TextureDescriptor *descriptor = texture->CreateDescriptor();
// 		if(descriptor)
// 		{
// 			if(fileFormat == PVR_FILE)
// 			{
// 				PVRConverter::Instance()->ConvertPngToPvr(descriptor->GetSourceTexturePathname(), *descriptor);
// 			}
// 			else if(fileFormat == DXT_FILE)
// 			{
// 
// 			}
// 
// 			descriptor->UpdateDateAndCrcForFormat(fileFormat);
// 			descriptor->Save();
// 			SafeRelease(descriptor);
// 		}
// 	}
}

void SceneDataManager::TextureReloadAll(DAVA::ImageFileFormat asFile)
{
	Map<String, Texture *> textures;
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		EnumerateTextures((*it)->GetScene(), textures);
	}

	Map<String, Texture *>::const_iterator endItTextures = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endItTextures; ++it)
	{
		TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(it->first);
		if(descriptor)
		{
			Texture *newTexture = TextureReload(descriptor, it->second, asFile);
			SafeRelease(descriptor);
		}
	}
}

DAVA::Texture * SceneDataManager::TextureReload(const TextureDescriptor *descriptor, DAVA::Texture *prevTexture, DAVA::ImageFileFormat asFile)
{
	if(!descriptor)
		return NULL;
	
	Texture *workingTexture = prevTexture;
	if(workingTexture == Texture::GetPinkPlaceholder())
	{
		//Create texture from descriptor pathname and real image file
		workingTexture = Texture::CreateFromFile(descriptor->pathname);
		RestoreTexture(descriptor->pathname, workingTexture);

		DVASSERT_MSG(1 < workingTexture->GetRetainCount(), "Can be more than 1");
		workingTexture->Release();

		if(workingTexture == Texture::GetPinkPlaceholder())
		{
			return workingTexture;
		}
	}

	//apply descriptor parameters
	workingTexture->ReloadAs((ImageFileFormat)asFile, descriptor);
	return workingTexture;
}

void SceneDataManager::RestoreTexture( const DAVA::String &descriptorPathname, DAVA::Texture *texture )
{
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		(*it)->RestoreTexture(descriptorPathname, texture);
	}
}

void SceneDataManager::EnumerateMaterials(DAVA::SceneNode *forNode, Vector<Material *> &materials)
{
	if(forNode)
	{
		forNode->GetDataNodes(materials);
	}
}

void SceneDataManager::SceneNodeSelectedInSceneGraph(SceneNode* node)
{
	SceneData *activeScene = SceneGetActive();
	
	
	activeScene->SceneNodeSelectedInGraph(node);
}

void SceneDataManager::RefreshParticlesLayer(DAVA::ParticleLayer* layer)
{
	emit SceneGraphNeedRefreshLayer(layer);
}

void SceneDataManager::UpdateParticleSprites()
{
	ParticlesEditorSpritePackerHelper::UpdateParticleSprites();
}

