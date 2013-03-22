#include "ToolsCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtMainWindowHandler.h"


using namespace DAVA;

//Show/Hide Materials
CommandMaterials::CommandMaterials()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandMaterials::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->MaterialsTriggered();
    }
}


//Show/Hide Texture Converter
CommandTextureConverter::CommandTextureConverter()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandTextureConverter::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
		// Replaced with Qt
		// TODO:
		// remove this
		// 
        // screen->TextureConverterTriggered();
    }
}

//Show settings
CommandSettings::CommandSettings()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSettings::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ShowSettings();
    }
}


//Beast
CommandBeast::CommandBeast()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandBeast::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ProcessBeast();
    }
}


//Ruler Tool
CommandRulerTool::CommandRulerTool()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandRulerTool::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->RulerToolTriggered();
    }
    
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RebuildSceneGraph();
    
    QtMainWindowHandler::Instance()->ShowStatusBarMessage(activeScene->GetScenePathname());
}


