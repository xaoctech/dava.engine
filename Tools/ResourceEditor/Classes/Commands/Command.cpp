#include "Command.h"

using namespace DAVA;


Command::Command(eCommandType _type)
    :   BaseObject()
    ,   commandType(_type)
    ,   commandState(STATE_VALID)
	,	commandName("")
{
    RegisterPointerType<Command *>(String("Command *"));
}

Command::~Command()
{
	
}


MultiCommand::MultiCommand(eCommandType _type)
:	Command(_type)
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