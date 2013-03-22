#include "VisibilityCheckToolCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/GUIState.h"
#include "../Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "../SceneEditor/EditorBodyControl.h"

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
    FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL,
													QString("Save texture"),
													QString(currentPath.GetAbsolutePathname().c_str()),
													QString("PNG image (*.png)"));

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(selectedPathname.IsInitalized())
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


CommandPlacePointVisibilityTool::CommandPlacePointVisibilityTool(const Vector2& newVisibilityPoint, const Vector2& oldVisibilityPoint, bool oldPointIsSet, Image* oldImage)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
,	point(newVisibilityPoint)
,	oldPoint(oldVisibilityPoint)
,	oldPointIsSet(oldPointIsSet)
{
	commandName = "Place Visibility Point";

	this->oldImage = SafeRetain(oldImage);
}

CommandPlacePointVisibilityTool::~CommandPlacePointVisibilityTool()
{
	SafeRelease(oldImage);
}

void CommandPlacePointVisibilityTool::Execute()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->SetVisibilityPoint(point);
	else
		SetState(STATE_INVALID);
}

void CommandPlacePointVisibilityTool::Cancel()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->RestorePointState(oldPoint, oldPointIsSet, oldImage);
}

LandscapeEditorVisibilityCheckTool* CommandPlacePointVisibilityTool::GetEditor()
{
	LandscapeEditorVisibilityCheckTool* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorVisibilityCheckTool*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL));
	}

	return editor;
}

CommandPlaceAreaVisibilityTool::CommandPlaceAreaVisibilityTool(const Vector2& areaPoint, uint32 areaSize, Image* oldImage)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT),
	point(areaPoint),
	size(areaSize),
	redoImage(NULL)
{
	commandName = "Place Visibility Area";

	this->oldImage = SafeRetain(oldImage);
}

CommandPlaceAreaVisibilityTool::~CommandPlaceAreaVisibilityTool()
{
	SafeRelease(oldImage);
}

void CommandPlaceAreaVisibilityTool::Execute()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
	{
		if (!redoImage)
		{
			editor->SetVisibilityArea(point, size);
			redoImage = editor->StoreAreaState();
		}
		else
			editor->RestoreAreaState(redoImage);
	}
	else
		SetState(STATE_INVALID);
}

void CommandPlaceAreaVisibilityTool::Cancel()
{
	LandscapeEditorVisibilityCheckTool* editor = GetEditor();
	if (editor)
		editor->RestoreAreaState(oldImage);
}

LandscapeEditorVisibilityCheckTool* CommandPlaceAreaVisibilityTool::GetEditor()
{
	LandscapeEditorVisibilityCheckTool* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorVisibilityCheckTool*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_VISIBILITY_CHECK_TOOL));
	}

	return editor;
}
