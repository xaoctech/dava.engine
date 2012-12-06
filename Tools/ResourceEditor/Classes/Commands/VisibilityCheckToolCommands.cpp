#include "VisibilityCheckToolCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/GUIState.h"
#include "../Qt/Main/QtUtils.h"
#include <QFileDialog>

CommandToggleVisibilityTool::CommandToggleVisibilityTool()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandToggleVisibilityTool::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		screen->VisibilityToolTriggered();
		GUIState::Instance()->SetNeedUpdatedToolsMenu(true);
		GUIState::Instance()->SetNeedUpdatedToolbar(true);
	}
}

CommandSetAreaVisibilityTool::CommandSetAreaVisibilityTool()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandSetAreaVisibilityTool::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		screen->VisibilityToolSetArea();
	}
}

CommandSetPointVisibilityTool::CommandSetPointVisibilityTool()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandSetPointVisibilityTool::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		screen->VisibilityToolSetPoint();
	}
}

CommandSaveTextureVisibilityTool::CommandSaveTextureVisibilityTool()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void CommandSaveTextureVisibilityTool::Execute()
{
    String currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL,
													QString("Save texture"),
													QString(currentPath.c_str()),
													QString("PNG image (*.png)"));

	String selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.length() > 0)
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->VisibilityToolSaveTexture(selectedPathname);
		}
	}
}

CommandChangeAreaSizeVisibilityTool::CommandChangeAreaSizeVisibilityTool(uint32 newSize)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT),
	size(newSize)
{
}

void CommandChangeAreaSizeVisibilityTool::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		screen->VisibilityToolSetAreaSize(size);
	}
}