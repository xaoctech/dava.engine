#include "LibraryCommands.h"

#include "../Qt/SceneData.h"
#include "../Qt/SceneDataManager.h"

#include "../SceneEditor/SceneEditorScreenMain.h"


#include "../Collada/ColladaConvert.h"


#include "DAVAEngine.h"

using namespace DAVA;

LibraryCommand::LibraryCommand(const DAVA::String &pathname, eCommandType _type)
    :   Command(_type)
    ,   filePathname(pathname)
{
}

bool LibraryCommand::IsExtensionCorrect(const DAVA::String &extenstionToChecking)
{
    String extension = FileSystem::Instance()->GetExtension(filePathname);
    return (0 == CompareStrings(extension, extenstionToChecking));
}



//Add scene to current tab
CommandAddScene::CommandAddScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, Command::COMMAND_UNDO_REDO)
{
}


void CommandAddScene::Execute()
{
    DVASSERT(IsExtensionCorrect(String(".sc2")) && "Wrong extension");
    
    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->AddScene(filePathname);
}

void CommandAddScene::Cancel()
{
    DVASSERT(IsExtensionCorrect(String(".sc2")) && "Wrong extension");

    //TODO: need code here
}


//edit scene at new tab
CommandEditScene::CommandEditScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandEditScene::Execute()
{
    DVASSERT(IsExtensionCorrect(String(".sc2")) && "Wrong extension");
    
    String path, name;
    FileSystem::Instance()->SplitPath(filePathname, path, name);
    
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->AddBodyItem(StringToWString(name), true);
    }

    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->EditScene(filePathname);
}


//reload root node at current tab
CommandReloadScene::CommandReloadScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, COMMAND_UNDO_REDO)
{
}


void CommandReloadScene::Execute()
{
    DVASSERT(IsExtensionCorrect(String(".sc2")) && "Wrong extension");

    SceneData *sceneData = SceneDataManager::Instance()->GetActiveScene();
    sceneData->ReloadRootNode(filePathname);
}

void CommandReloadScene::Cancel()
{
    DVASSERT(IsExtensionCorrect(String(".sc2")) && "Wrong extension");
    //TODO: need code here
}


//convert from dae to sc2
CommandConvertScene::CommandConvertScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandConvertScene::Execute()
{
    DVASSERT(IsExtensionCorrect(String(".dae")) && "Wrong extension");

    ConvertDaeToSce(filePathname);
    
    // load sce to scene object
    String path = FileSystem::Instance()->ReplaceExtension(filePathname, ".sce");
    Scene * scene = new Scene();

    SceneNode *rootNode = scene->GetRootNode(path);
    scene->AddNode(rootNode);
    
    scene->BakeTransforms();
    
    // Export to *.sc2
    path = FileSystem::Instance()->ReplaceExtension(path, ".sc2");
    SceneFileV2 * file = new SceneFileV2();
    file->EnableDebugLog(true);
    file->SaveScene(path.c_str(), scene);
    SafeRelease(file);
    
    SafeRelease(scene);
}
