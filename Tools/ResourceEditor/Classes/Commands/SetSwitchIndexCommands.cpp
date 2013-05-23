#include "SetSwitchIndexCommands.h"

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/QtUtils.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"


CommandToggleSetSwitchIndex::CommandToggleSetSwitchIndex(uint32 value, SetSwitchIndexHelper::eSET_SWITCH_INDEX state)
:   CommandEntityModification(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_TOGGLE_SET_SWITCH_INDEX)
{
    this->value = value;
	this->swtichState = state;
}

void CommandToggleSetSwitchIndex::Execute()
{
	SetSwitchIndexHelper::ProcessSwitchIndexUpdate(this->value, this->swtichState);

}
