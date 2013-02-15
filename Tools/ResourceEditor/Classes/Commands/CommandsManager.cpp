#include "CommandsManager.h"
#include "Command.h"

#include "../Qt/Main/QtMainWindowHandler.h"

using namespace DAVA;

CommandsManager::CommandsManager()
:	activeQueue(NULL)
{
}

CommandsManager::~CommandsManager()
{
	ClearAllQueues();
}

void CommandsManager::ClearAllQueues()
{
	for (QUEUE_LIST::iterator it = queueList.begin(); it != queueList.end(); ++it)
	{
		ClearQueue((*it).second);
		SafeDelete((*it).second);
	}
	queueList.clear();
}

void CommandsManager::ChangeQueue(void *scene)
{
	DVASSERT(scene);

	QUEUE_LIST::iterator it = queueList.find(scene);

	if (it != queueList.end())
	{
		activeQueue = (*it).second;
	}
	else
	{
		UndoQueue* newQueue = new UndoQueue(scene);
		queueList[scene] = newQueue;
		activeQueue = newQueue;
	}
}

void CommandsManager::ClearQueue(UndoQueue* queue)
{
	if (!queue)
		return;

	UndoQueue* q = queue;
	if (!q)
		q = activeQueue;

	q->commandIndex = -1;
	for_each(q->commands.begin(),
			 q->commands.end(),
			 SafeRelease<Command>);
	q->commands.clear();
}

void CommandsManager::ClearQueueTail()
{
	if ((activeQueue->commandIndex >= -1) &&
		(activeQueue->commandIndex < (int32)activeQueue->commands.size()))
	{
		int32 newCount = activeQueue->commandIndex + 1;
		for_each(activeQueue->commands.begin() + newCount,
				 activeQueue->commands.end(),
				 SafeRelease<Command>);
		activeQueue->commands.resize(newCount);
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
				activeQueue->commands.push_back(SafeRetain(command));
				++activeQueue->commandIndex;
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
	if ((activeQueue->commandIndex >= 0) &&
		(activeQueue->commandIndex < (int32)activeQueue->commands.size()))
	{
		//TODO: need check state?
		activeQueue->commands[activeQueue->commandIndex]->Cancel();
		--activeQueue->commandIndex;

		QtMainWindowHandler::Instance()->UpdateUndoActionsState();
	}
}

void CommandsManager::Redo()
{
	if ((activeQueue->commandIndex >= -1) &&
		(activeQueue->commandIndex < (int32)activeQueue->commands.size() - 1))
	{
		//TODO: need check state?
		++activeQueue->commandIndex;
		activeQueue->commands[activeQueue->commandIndex]->Execute();
	}
}

int32 CommandsManager::GetUndoQueueLength()
{
	return activeQueue->commandIndex + 1;
}

int32 CommandsManager::GetRedoQueueLength()
{
	return activeQueue->commands.size() - activeQueue->commandIndex - 1;
}

String CommandsManager::GetUndoCommandName()
{
	if ((activeQueue->commandIndex >= 0) &&
		(activeQueue->commandIndex < (int32)activeQueue->commands.size()))
	{
		return activeQueue->commands[activeQueue->commandIndex]->commandName;
	}

	return "";
}

String CommandsManager::GetRedoCommandName()
{
	if ((activeQueue->commandIndex >= -1) &&
		(activeQueue->commandIndex < (int32)activeQueue->commands.size() - 1))
	{
		return activeQueue->commands[activeQueue->commandIndex + 1]->commandName;
	}

	return "";
}
