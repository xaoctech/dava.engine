#include "ToolsCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"

#include "../Qt/GUIState.h"

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
        screen->TextureConverterTriggered();
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
}

