#include "SceneDataManager.h"

#include "Main/SceneGraphModel.h"
#include "Main/LibraryModel.h"

#include "../SceneEditor/SceneValidator.h"
#include "../SceneEditor/PVRConverter.h"

#include <QTreeView>

using namespace DAVA;

SceneDataManager::SceneDataManager()
    :   currentScene(NULL)
    ,   sceneGraphView(NULL)
    ,   libraryView(NULL)
    ,   libraryModel(NULL)
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

void SceneDataManager::SetActiveScene(EditorScene *scene)
{
    if(currentScene)
    {
        currentScene->Deactivate();
    }
    
    
    currentScene = FindDataForScene(scene);
    DVASSERT(currentScene && "There is no current scene. Something wrong.");
    
    DVASSERT(sceneGraphView && "QTreeView not initialized");
    currentScene->RebuildSceneGraph();
    currentScene->Activate(sceneGraphView, libraryView, libraryModel);

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


SceneData * SceneDataManager::GetActiveScene()
{
	return currentScene;
}

SceneData *SceneDataManager::GetLevelScene()
{
    if(0 < scenes.size())
    {
        return scenes.front();
    }
    
    return NULL;
}

DAVA::SceneNode* SceneDataManager::GetSelectedNode(SceneData *scene)
{
	DAVA::SceneNode *node = NULL;

	if(NULL != scene)
	{
		node = scene->GetSelectedNode();
	}

	return node;
}

EditorScene * SceneDataManager::RegisterNewScene()
{
    SceneData *data = new SceneData();
    data->CreateScene(true);

    scenes.push_back(data);
    
	connect(data, SIGNAL(SceneChanged(EditorScene *)), this, SLOT(InSceneData_SceneChanged(EditorScene *)));
	connect(data, SIGNAL(SceneNodeSelected(DAVA::SceneNode *)), this, SLOT(InSceneData_SceneNodeSelected(DAVA::SceneNode *)));
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

DAVA::int32 SceneDataManager::ScenesCount()
{
    return (int32)scenes.size();
}

SceneData *SceneDataManager::GetScene(DAVA::int32 index)
{
    DVASSERT((0 <= index) && (index < (int32)scenes.size()));
    
    DAVA::List<SceneData *>::iterator it = scenes.begin();
    std::advance(it, index);
    
    return *it;
}


void SceneDataManager::SetSceneGraphView(QTreeView *view)
{
    sceneGraphView = view;
}

void SceneDataManager::SetLibraryView(QTreeView *view)
{
    libraryView = view;
}

void SceneDataManager::SetLibraryModel(LibraryModel *model)
{
    libraryModel = model;
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

void SceneDataManager::EnumerateTextures( DAVA::Map<DAVA::String, DAVA::Texture *> &textures )
{
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		(*it)->EnumerateTextures(textures);
	}
}


void SceneDataManager::CompressNotCompressedTextures()
{
	Map<String, Texture *> textures;
	EnumerateTextures(textures);

	List<Texture *>texturesForPVRCompression;
	List<Texture *>texturesForDXTCompression;

	Map<String, Texture *>::const_iterator endIt = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endIt; ++it)
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

void SceneDataManager::ReloadTextures( int32 asFile )
{
	Map<String, Texture *> textures;
	EnumerateTextures(textures);

	Map<String, Texture *>::const_iterator endIt = textures.end();
	for(Map<String, Texture *>::const_iterator it = textures.begin(); it != endIt; ++it)
	{
		Texture *newTexture = ReloadTexture(it->first, it->second, asFile);
	}
}

DAVA::Texture * SceneDataManager::ReloadTexture( const DAVA::String &descriptorPathname, DAVA::Texture *prevTexture, int32 asFile )
{
	if(prevTexture == Texture::GetPinkPlaceholder())
	{
		Texture *newTexture = Texture::CreateFromFile(descriptorPathname);
		RestoreTexture(descriptorPathname, newTexture);

		DVASSERT_MSG(1 < newTexture->GetRetainCount(), "Can be more than 1");
		newTexture->Release();

		return newTexture;
	}

	TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
	if(descriptor)
	{
		prevTexture->ReloadAs((ImageFileFormat)asFile, descriptor);
		SafeRelease(descriptor);
	}

	return prevTexture;
}

void SceneDataManager::RestoreTexture( const DAVA::String &descriptorPathname, DAVA::Texture *texture )
{
	List<SceneData *>::const_iterator endIt = scenes.end();
	for(List<SceneData *>::const_iterator it = scenes.begin(); it != endIt; ++it)
	{
		(*it)->RestoreTexture(descriptorPathname, texture);
	}
}


