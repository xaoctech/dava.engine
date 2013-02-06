#include "CustomColorCommands.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/GUIState.h"
#include <QFileDialog>
#include "EditorBodyControl.h"

CommandToggleCustomColors::CommandToggleCustomColors()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    
}

void CommandToggleCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->CustomColorsTriggered();
        GUIState::Instance()->SetNeedUpdatedToolsMenu(true);//TODO
        GUIState::Instance()->SetNeedUpdatedToolbar(true);
    }

}

CommandSaveTextureCustomColors::CommandSaveTextureCustomColors()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    
}

void CommandSaveTextureCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(!screen)
		return;

	String selectedPathname = screen->CustomColorsGetCurrentSaveFileName();

	if(selectedPathname.empty())
	{
		String sceneFilePath = screen->CurrentScenePathname();
		String sceneFileName = "";
		FileSystem::SplitPath(sceneFilePath, selectedPathname, sceneFileName);
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save texture"), QString(selectedPathname.c_str()), QString("PNG image (*.png)"));

	selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.empty())
		screen->CustomColorsSaveTexture(selectedPathname);
}

CommandLoadTextureCustomColors::CommandLoadTextureCustomColors()
:	Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandLoadTextureCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(!screen)
		return;

	String currentPath = screen->CustomColorsGetCurrentSaveFileName();

	if(currentPath.empty())
	{
		String sceneFilePath = screen->CurrentScenePathname();
		String sceneFileName = "";
		FileSystem::SplitPath(sceneFilePath, currentPath, sceneFileName);
	}

	String selectedPathname = GetOpenFileName(String("Load texture"), currentPath, String("PNG image (*.png)"));
	if(!selectedPathname.empty())
	{
		screen->CustomColorsLoadTexture(selectedPathname);
	}
}

CommandChangeBrushSizeCustomColors::CommandChangeBrushSizeCustomColors(uint32 newSize)
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT),
    size(newSize)
{    
}

void CommandChangeBrushSizeCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->CustomColorsSetRadius(size);
    }
}

CommandChangeColorCustomColors::CommandChangeColorCustomColors(uint32 newColorIndex)
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT),
    colorIndex(newColorIndex)
{
    
}

void CommandChangeColorCustomColors::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
		screen->CustomColorsSetColor(colorIndex);
    }
}

CommandDrawCustomColors::CommandDrawCustomColors()
:	Command(COMMAND_UNDO_REDO)
{
	redoImage = NULL;

	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor)
		editor->StoreState(&undoImage);
}

CommandDrawCustomColors::~CommandDrawCustomColors()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
}

void CommandDrawCustomColors::Execute()
{
	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	if (redoImage == NULL)
	{
		editor->StoreState(&redoImage);
	}
	else
	{
		editor->RestoreState(redoImage);
	}
}

void CommandDrawCustomColors::Cancel()
{
	LandscapeEditorCustomColors* editor = GetEditor();
	if (editor)
		editor->RestoreState(undoImage);
}

LandscapeEditorCustomColors* CommandDrawCustomColors::GetEditor()
{
	LandscapeEditorCustomColors* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorCustomColors*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_CUSTOM_COLORS));
	}

	return editor;
}