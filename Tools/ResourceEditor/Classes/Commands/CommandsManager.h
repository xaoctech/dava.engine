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
	void ExecuteAndRelease(Command* command);
    void Undo();
    void Redo();

	DAVA::int32 GetUndoQueueLength();
	DAVA::int32 GetRedoQueueLength();

	DAVA::String GetUndoCommandName();
	DAVA::String GetRedoCommandName();

	void ChangeQueue(void* scene);
	void SceneReleased(void* scene);

private:
	struct UndoQueue
	{
		DAVA::Vector<Command*> commands;
		DAVA::int32 commandIndex;
		void* activeScene;

		UndoQueue(void* scene)
		:	commandIndex(-1)
		,	activeScene(scene)
		{ commands.reserve(UNDO_QUEUE_SIZE); };
	};

	typedef DAVA::Map<void*, UndoQueue*> QUEUE_MAP;

    void ClearQueue(UndoQueue* queue = NULL);
    void ClearQueueTail();
	void ClearAllQueues();

	QUEUE_MAP queueMap;
	UndoQueue* activeQueue;
};



#endif // __COMMANDS_MANAGER_H__