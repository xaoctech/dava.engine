#include "TilemapEditorCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/GUIState.h"
#include "EditorBodyControl.h"

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


CommandDrawTilemap::CommandDrawTilemap()
:	Command(COMMAND_UNDO_REDO)
{
	redoImage = NULL;

	LandscapeEditorColor* editor = GetEditor();
	if (editor)
		editor->StoreState(&undoImage);
}

CommandDrawTilemap::~CommandDrawTilemap()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
}

void CommandDrawTilemap::Execute()
{
	LandscapeEditorColor* editor = GetEditor();
	if (editor == NULL)
		return;

	if (redoImage == NULL)
	{
		editor->StoreState(&redoImage);
	}
	else
	{
		editor->RestoreState(redoImage);
	}
}

void CommandDrawTilemap::Cancel()
{
	LandscapeEditorColor* editor = GetEditor();
	if (editor)
		editor->RestoreState(undoImage);
}

LandscapeEditorColor* CommandDrawTilemap::GetEditor()
{
	LandscapeEditorColor* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorColor*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_COLOR_MAP));
	}

	return editor;
}