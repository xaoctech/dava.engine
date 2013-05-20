#include "Command.h"

using namespace DAVA;


Command::Command(eCommandType _type, CommandList::eCommandId id)
    :   BaseObject()
    ,   commandType(_type)
    ,   commandState(STATE_VALID)
	,	commandName("")
	,	commandId(id)
{
    RegisterPointerType<Command *>(String("Command *"));
}

Command::~Command()
{
	
}

DAVA::Set<DAVA::Entity*> Command::GetAffectedEntities()
{
	Set<Entity*> entities;
	return entities;
}


MultiCommand::MultiCommand(eCommandType _type, CommandList::eCommandId id)
:	Command(_type, id)
{
}

void MultiCommand::ExecuteInternal(Command* command)
{
	DVASSERT(command);
	command->Execute();
}

void MultiCommand::CancelInternal(Command* command)
{
	DVASSERT(command);
	command->Cancel();
}

Command::eCommandState MultiCommand::GetInternalCommandState(Command* command)
{
	DVASSERT(command);
	return command->State();
}

DAVA::Set<DAVA::Entity*> MultiCommand::GetAffectedEntitiesInternal(Command* command)
{
	DVASSERT(command);
	return command->GetAffectedEntities();
}