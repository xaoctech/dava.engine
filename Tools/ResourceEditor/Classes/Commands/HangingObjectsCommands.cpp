#include "HangingObjectsCommands.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "../SceneEditor/EditorBodyControl.h"
#include "../Qt/DockHangingObjects/HangingObjectsHelper.h"


CommandToggleHangingObjects::CommandToggleHangingObjects(float value, bool isEnabled)
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->value = value;
	this->isEnabled = isEnabled;
}

void CommandToggleHangingObjects::Execute()
{
	HangingObjectsHelper::ProcessHangingObjectsUpdate(value, isEnabled);
}
