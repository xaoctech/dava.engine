#include "CustomColorCommands.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/QtUtils.h"
#include <QFileDialog>
#include "../Qt/GUIState.h"

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
    String currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save texture"), QString(currentPath.c_str()), QString("PNG image (*.png)"));
    
	String selectedPathname = PathnameToDAVAStyle(filePath);
//	String selectedPathname = filePath.toStdString();

	if(selectedPathname.length() > 0)
	{
		SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
		if(screen)
		{
			screen->CustomColorsSaveTexture(selectedPathname);
		}
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