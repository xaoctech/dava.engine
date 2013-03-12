#include "EditCommands.h"
#include "CommandsManager.h"

UndoCommand::UndoCommand()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void UndoCommand::Execute()
{
	CommandsManager::Instance()->Undo();
}


RedoCommand::RedoCommand()
:	Command(COMMAND_WITHOUT_UNDO_EFFECT)
{
}

void RedoCommand::Execute()
{
	CommandsManager::Instance()->Redo();
}
