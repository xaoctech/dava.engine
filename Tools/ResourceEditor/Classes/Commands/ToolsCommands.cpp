#include "ToolsCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"

#include "../Qt/GUIState.h"
#include "../Qt/SceneData.h"
#include "../Qt/SceneDataManager.h"
#include "../Qt/QtMainWindowHandler.h"


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

//Show/Hide Heightmap Editor
CommandHeightmapEditor::CommandHeightmapEditor()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandHeightmapEditor::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->HeightmapTriggered();
        GUIState::Instance()->SetNeedUpdatedToolsMenu(true);
        GUIState::Instance()->SetNeedUpdatedToolbar(true);
    }
    
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->RebuildSceneGraph();
}


//Show/Hide Tilemap Editor
CommandTilemapEditor::CommandTilemapEditor()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandTilemapEditor::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->TilemapTriggered();
        GUIState::Instance()->SetNeedUpdatedToolsMenu(true);
        GUIState::Instance()->SetNeedUpdatedToolbar(true);
    }

//    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
//    activeScene->RebuildSceneGraph();
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

//Bake active scene
CommandBakeScene::CommandBakeScene()
:   Command(Command::COMMAND_UNDO_REDO)
{
}


void CommandBakeScene::Execute()
{
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->BakeScene();
}

void CommandBakeScene::Cancel()
{
    //TODO: write code
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
    
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->RebuildSceneGraph();
    
    QtMainWindowHandler::Instance()->ShowStatusBarMessage(activeScene->GetScenePathname());
}


