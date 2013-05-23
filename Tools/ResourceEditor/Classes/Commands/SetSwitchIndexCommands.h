#ifndef __RESOURCE_EDITOR_SWITCH_INDEX_COMMANDS_H__
#define __RESOURCE_EDITOR_SWITCH_INDEX_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "../Constants.h"
#include "../Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"
#include "EditorBodyControlCommands.h"

class CommandToggleSetSwitchIndex: public CommandEntityModification
{
public:
	DAVA_DEPRECATED(CommandToggleSetSwitchIndex(DAVA::uint32 value, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX state));
	
protected:
	DAVA::uint32	value;
	DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX	swtichState;

    virtual void Execute();
};

#endif // #ifndef __RESOURCE_EDITOR_SWITCH_INDEX_COMMANDS_H__