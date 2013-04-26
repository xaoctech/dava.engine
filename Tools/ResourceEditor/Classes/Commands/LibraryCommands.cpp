#include "LibraryCommands.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtMainWindowHandler.h"
#include "../Qt/Main/QtUtils.h"


#include "../SceneEditor/SceneEditorScreenMain.h"


#include "../Collada/ColladaConvert.h"

#include "../SceneEditor/SceneValidator.h"
#include "../SceneEditor/EditorSettings.h"

#include "DAVAEngine.h"

using namespace DAVA;

LibraryCommand::LibraryCommand(const DAVA::String &pathname, eCommandType _type)
    :   Command(_type)
    ,   filePathname(FileSystem::Instance()->GetCanonicalPath(pathname))
{
}

bool LibraryCommand::CheckExtension(const DAVA::String &extenstionToChecking)
{
    String extension = FileSystem::Instance()->GetExtension(filePathname);
    return (0 == CompareCaseInsensitive(extension, extenstionToChecking));
}



//Add scene to current tab
CommandAddScene::CommandAddScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, Command::COMMAND_UNDO_REDO)
{
	commandName = "Add Scene";
}


void CommandAddScene::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    SceneDataManager::Instance()->AddScene(filePathname);
}

void CommandAddScene::Cancel()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");

    //TODO: need code here
}


//edit scene at new tab
CommandEditScene::CommandEditScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandEditScene::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    
    String path, name;
    FileSystem::Instance()->SplitPath(filePathname, path, name);
    
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->AddBodyItem(StringToWString(name), true);
    }

    SceneDataManager::Instance()->EditActiveScene(filePathname);

    QtMainWindowHandler::Instance()->ShowStatusBarMessage(filePathname);
}


//reload root node at current tab
CommandReloadScene::CommandReloadScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, COMMAND_UNDO_REDO)
{
	commandName = "Reload Scene";
}


void CommandReloadScene::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    SceneDataManager::Instance()->ReloadScene(filePathname, filePathname);
}

void CommandReloadScene::Cancel()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    //TODO: need code here
}


//reload root node at current tab
CommandReloadEntityFrom::CommandReloadEntityFrom(const DAVA::String &pathname)
:   LibraryCommand(pathname, COMMAND_UNDO_REDO)
{
	commandName = "Reload Entity From";
    fromPathname = String("");
}


void CommandReloadEntityFrom::Execute()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    
    String path, name;
    FileSystem::Instance()->SplitPath(filePathname, path, name);
    
    fromPathname = GetOpenFileName(String("Select Scene File"), (path.c_str()), String("Scene File (*.sc2)"));
    if(fromPathname.empty())
    {
        return;
    }
    
    SceneDataManager::Instance()->ReloadScene(filePathname, fromPathname);
}

void CommandReloadEntityFrom::Cancel()
{
    DVASSERT(CheckExtension(String(".sc2")) && "Wrong extension");
    //TODO: need code here
}



//convert from dae to sc2
CommandConvertScene::CommandConvertScene(const DAVA::String &pathname)
    :   LibraryCommand(pathname, COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandConvertScene::Execute()
{
    DVASSERT(CheckExtension(String(".dae")) && "Wrong extension");
    SceneValidator::Instance()->CreateDefaultDescriptors(EditorSettings::Instance()->GetDataSourcePath());
    
    eColladaErrorCodes code = ConvertDaeToSce(filePathname);
    if(code == COLLADA_OK)
    {
        // load sce to scene object
        String path = FileSystem::Instance()->ReplaceExtension(filePathname, ".sce");
        Scene * scene = new Scene();
        
        Entity *rootNode = scene->GetRootNode(path);
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
    else if(code == COLLADA_ERROR_OF_ROOT_NODE)
    {
        ShowErrorDialog(String("Can't convert from DAE. Looks like one of materials has same name as root node."));
    }
    else
    {
        ShowErrorDialog(String("Can't convert from DAE."));
    }
}

