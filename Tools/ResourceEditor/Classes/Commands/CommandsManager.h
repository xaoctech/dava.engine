#ifndef __COMMANDS_MANAGER_H__
#define __COMMANDS_MANAGER_H__

#include "DAVAEngine.h"

class Command;
class CommandsManager: public DAVA::Singleton<CommandsManager>
{
    static const DAVA::int32 UNDO_QUEUE_SIZE = 50;
    
public:	
	CommandsManager();
	virtual ~CommandsManager();
	
    void Execute(Command *command);
    void Undo();
    void Redo();

	DAVA::int32 GetUndoQueueLength();
	DAVA::int32 GetRedoQueueLength();

	DAVA::String GetUndoCommandName();
	DAVA::String GetRedoCommandName();
private:

    void ClearQueue();
    void ClearQueueTail();
    
    DAVA::Vector<Command *> commandsQueue;
    DAVA::int32 currentCommandIndex;
};



#endif // __COMMANDS_MANAGER_H__