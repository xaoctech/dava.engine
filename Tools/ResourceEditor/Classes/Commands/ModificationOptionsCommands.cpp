#include "ModificationOptionsCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/EditorSettings.h"

ModificationPlaceOnLandCommand::ModificationPlaceOnLandCommand()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void ModificationPlaceOnLandCommand::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	screen->FindCurrentBody()->bodyControl->OnPlaceOnLandscape();
}

ModificationApplyCommand::ModificationApplyCommand(float32 x, float32 y, float32 z)
:	Command(COMMAND_WITHOUT_UNDO_EFFECT),
	x(x),
	y(y),
	z(z)
{
}

void ModificationApplyCommand::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	screen->FindCurrentBody()->bodyControl->ApplyTransform(x, y, z);
}

ModificationResetCommand::ModificationResetCommand()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void ModificationResetCommand::Execute()
{
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	screen->FindCurrentBody()->bodyControl->RestoreOriginalTransform();
}
