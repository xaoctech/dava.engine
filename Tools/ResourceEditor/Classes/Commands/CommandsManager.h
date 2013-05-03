#ifndef __COMMANDS_MANAGER_H__
#define __COMMANDS_MANAGER_H__

#include "DAVAEngine.h"

class Command;
class DAVA::Scene;

class CommandsManager: public DAVA::Singleton<CommandsManager>
{
    static const DAVA::int32 UNDO_QUEUE_SIZE = 10;
    
public:	
	CommandsManager();
	virtual ~CommandsManager();
	
    void Execute(Command *command, DAVA::Scene* forScene = NULL);
	void ExecuteAndRelease(Command* command, DAVA::Scene* forScene = NULL);
    void Undo(DAVA::Scene* forScene = NULL);
    void Redo(DAVA::Scene* forScene = NULL);

	DAVA::int32 GetUndoQueueLength(DAVA::Scene* forScene = NULL);
	DAVA::int32 GetRedoQueueLength(DAVA::Scene* forScene = NULL);

	DAVA::String GetUndoCommandName(DAVA::Scene* forScene = NULL);
	DAVA::String GetRedoCommandName(DAVA::Scene* forScene = NULL);

	void SceneReleased(DAVA::Scene* scene);

private:
	struct UndoQueue
	{
		DAVA::Vector<Command*> commands;
		DAVA::int32 commandIndex;
		DAVA::Scene* activeScene;

		UndoQueue(DAVA::Scene* scene)
		:	commandIndex(-1)
		,	activeScene(scene)
		{ commands.reserve(UNDO_QUEUE_SIZE); };
	};

	typedef DAVA::Map<DAVA::Scene*, UndoQueue*> QUEUE_MAP;

    void ClearQueue(UndoQueue* queue);
    void ClearQueueTail(UndoQueue* queue);
	void ClearAllQueues();
	UndoQueue* GetQueueForScene(DAVA::Scene* scene);

	void EmitCommandExecutedSignal(Command* command, DAVA::Scene* scene);

	QUEUE_MAP queueMap;
};



#endif // __COMMANDS_MANAGER_H__
