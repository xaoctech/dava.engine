#include "HangingObjectsCommands.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include "../Qt/Main/GUIState.h"
#include <QFileDialog>
#include "../SceneEditor/EditorBodyControl.h"
//#include "../Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"


CommandToggleHangingObjects::CommandToggleHangingObjects(float value)
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
    this->value = value;
}

void CommandToggleHangingObjects::Execute()
{
	//SetSwitchIndexHelper::ProcessSwitchIndexUpdate(this->value, this->swtichState);
	int k =0;
}
