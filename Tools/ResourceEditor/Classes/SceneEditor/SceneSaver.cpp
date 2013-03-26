#include "SceneSaver.h"
#include "SceneValidator.h"

#include "../Qt/Scene/SceneDataManager.h"

using namespace DAVA;

SceneSaver::SceneSaver()
{
}

SceneSaver::~SceneSaver()
{
    ReleaseTextures();
}


void SceneSaver::SetInFolder(const String &folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneSaver::SetOutFolder(const String &folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}


void SceneSaver::SaveFile(const String &fileName, Set<String> &errorLog)
{
    Logger::Info("[SceneSaver::SaveFile] %s", fileName.c_str());
    
    //Load scene with *.sc2
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(sceneUtils.dataSourceFolder + fileName);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
		Vector<Entity*> tempV;
		tempV.reserve((count));
        for(int32 i = 0; i < count; ++i)
        {
			tempV.push_back(rootNode->GetChild(i));
        }
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}
		
		SaveScene(scene, fileName, errorLog);
    }
	else
	{
		errorLog.insert(Format("[SceneSaver::SaveFile] Can't open file %s", fileName.c_str()));
	}

    SafeRelease(scene);
}

void SceneSaver::SaveScene(Scene *scene, const String &fileName, Set<String> &errorLog)
{
    DVASSERT(0 == texturesForSave.size())
    
    //Create destination folder
    String normalizedFileName = FileSystem::Instance()->GetCanonicalPath(fileName);

    String workingFile;
    FileSystem::SplitPath(normalizedFileName, sceneUtils.workingFolder, workingFile);
    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true); 

    scene->Update(0.1f);

    String oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, errorLog);

    texturesForSave.clear();
    SceneDataManager::EnumerateTextures(scene, texturesForSave);

    CopyTextures(scene, errorLog);
	ReleaseTextures();

	Landscape *landscape = EditorScene::GetLandscape(scene);
    if (landscape)
    {
        sceneUtils.CopyFile(landscape->GetHeightmapPathname(), errorLog);
    }

	CopyReferencedObject(scene, errorLog);

    //save scene to new place
    String scenePath, sceneName;
    FileSystem::Instance()->SplitPath(fileName, scenePath, sceneName);

    String tempSceneName = scenePath + FileSystem::Instance()->ReplaceExtension(sceneName, ".saved.sc2");
    
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    
    outFile->SaveScene(sceneUtils.dataSourceFolder + tempSceneName, scene);
    SafeRelease(outFile);

    FileSystem::Instance()->MoveFile(sceneUtils.dataSourceFolder + tempSceneName, sceneUtils.dataFolder + fileName);
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}


void SceneSaver::CopyTextures(DAVA::Scene *scene, Set<String> &errorLog)
{
    Map<String, Texture *>::const_iterator endIt = texturesForSave.end();
    Map<String, Texture *>::iterator it = texturesForSave.begin();
    for( ; it != endIt; ++it)
    {
        CopyTexture(it->first, errorLog);
    }
}

void SceneSaver::ReleaseTextures()
{
    texturesForSave.clear();
}

void SceneSaver::CopyTexture(const String &texturePathname, Set<String> &errorLog)
{
    String descriptorPathname = TextureDescriptor::GetDescriptorPathname(texturePathname);
    String pngPathname = TextureDescriptor::GetPathnameForFormat(texturePathname, PNG_FILE);

    sceneUtils.CopyFile(descriptorPathname, errorLog);
    sceneUtils.CopyFile(pngPathname, errorLog);
}

void SceneSaver::CopyReferencedObject( Entity *node, Set<String> &errorLog )
{
	KeyedArchive *customProperties = node->GetCustomProperties();
	if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
	{
		String path = customProperties->GetString("editor.referenceToOwner");
		sceneUtils.CopyFile(path, errorLog);
	}
	for (int i = 0; i < node->GetChildrenCount(); i++)
	{
		CopyReferencedObject(node->GetChild(i), errorLog);
	}

}


