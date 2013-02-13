#include "CommandsManager.h"
#include "Command.h"

#include "../Qt/Main/QtMainWindowHandler.h"

using namespace DAVA;

CommandsManager::CommandsManager()
    :   currentCommandIndex(-1)
{
    commandsQueue.reserve(UNDO_QUEUE_SIZE);
}

CommandsManager::~CommandsManager()
{
    ClearQueue();
}

void CommandsManager::ClearQueue()
{
    currentCommandIndex = -1;
    
	for_each(commandsQueue.begin(), commandsQueue.end(), SafeRelease<Command>);
    commandsQueue.clear();
}

void CommandsManager::ClearQueueTail()
{
    if((-1 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size()))
    {
        int32 newCount = currentCommandIndex + 1;
		for_each(commandsQueue.begin() + newCount, commandsQueue.end(), SafeRelease<Command>);
        
        commandsQueue.resize(newCount);
    }
}


void CommandsManager::Execute(Command *command)
{
    command->Execute();
    
    switch (command->Type()) 
    {
        case Command::COMMAND_WITHOUT_UNDO_EFFECT:
            break;
            
        case Command::COMMAND_UNDO_REDO:
            
            if(Command::STATE_VALID == command->State())
            {
                //TODO: VK: if need only UNDO_QUEUE_SIZE commands at queue you may add code here
                ClearQueueTail();
                commandsQueue.push_back(SafeRetain(command));
                ++currentCommandIndex;
            }
            break;

        case Command::COMMAND_CLEAR_UNDO_QUEUE:
            if(Command::STATE_VALID == command->State())
            {
                ClearQueue();
            }
            break;

        default:
            Logger::Warning("[CommandsManager::Execute] command type (%d) not processed", command->Type());
            break;
    }

	QtMainWindowHandler::Instance()->UpdateUndoActionsState();
}

void CommandsManager::Undo()
{
    if((0 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size()))
    {
        //TODO: need check state?
        commandsQueue[currentCommandIndex]->Cancel();
        --currentCommandIndex;

		QtMainWindowHandler::Instance()->UpdateUndoActionsState();
    }
}

void CommandsManager::Redo()
{
    if((-1 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size() - 1))
    {
        //TODO: need check state?
        ++currentCommandIndex;
        commandsQueue[currentCommandIndex]->Execute();
    }
}

int32 CommandsManager::GetUndoQueueLength()
{
	return currentCommandIndex + 1;
}

int32 CommandsManager::GetRedoQueueLength()
{
	return commandsQueue.size() - currentCommandIndex - 1;
}

String CommandsManager::GetUndoCommandName()
{
	if((0 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size()))
	{
		return commandsQueue[currentCommandIndex]->commandName;
	}

	return "";
}

String CommandsManager::GetRedoCommandName()
{
	if((-1 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size() - 1))
	{
		return commandsQueue[currentCommandIndex + 1]->commandName;
	}

	return "";
}
