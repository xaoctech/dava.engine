#include "CommandsManager.h"
#include "Command.h"

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
    
    int32 queueSize = (int32)commandsQueue.size();
    for(int32 i = 0; i < queueSize; ++i)
    {
        SafeRelease(commandsQueue[i]);
    }
    commandsQueue.clear();
}

void CommandsManager::ClearQueueTail()
{
    if((0 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size()))
    {
        int32 newCount = currentCommandIndex + 1;
        int32 queueSize = (int32)commandsQueue.size();
        for(int32 i = newCount; i < queueSize; ++i)
        {
            SafeRelease(commandsQueue[i]);
        }
        
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
}

void CommandsManager::Undo()
{
    if((0 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size()))
    {
        //TODO: need check state?
        commandsQueue[currentCommandIndex]->Cancel();
        --currentCommandIndex;
    }
}

void CommandsManager::Redo()
{
    if((0 <= currentCommandIndex) && (currentCommandIndex < (int32)commandsQueue.size() - 1))
    {
        //TODO: need check state?
        ++currentCommandIndex;
        commandsQueue[currentCommandIndex]->Execute();
    }
}


