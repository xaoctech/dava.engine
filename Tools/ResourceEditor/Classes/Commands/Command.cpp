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




