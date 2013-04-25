#ifndef __COMMANDS_MANAGER_H__
#define __COMMANDS_MANAGER_H__

#include "DAVAEngine.h"

class Command;
class SceneData;

class CommandsManager: public DAVA::Singleton<CommandsManager>
{
    static const DAVA::int32 UNDO_QUEUE_SIZE = 10;
    
public:	
	CommandsManager();
	virtual ~CommandsManager();
	
    void Execute(Command *command, SceneData* forScene = NULL);
	void ExecuteAndRelease(Command* command, SceneData* forScene = NULL);
    void Undo(SceneData* forScene = NULL);
    void Redo(SceneData* forScene = NULL);

	DAVA::int32 GetUndoQueueLength(SceneData* forScene = NULL);
	DAVA::int32 GetRedoQueueLength(SceneData* forScene = NULL);

	DAVA::String GetUndoCommandName(SceneData* forScene = NULL);
	DAVA::String GetRedoCommandName(SceneData* forScene = NULL);

	void SceneReleased(SceneData* scene);

private:
	struct UndoQueue
	{
		DAVA::Vector<Command*> commands;
		DAVA::int32 commandIndex;
		SceneData* activeScene;

		UndoQueue(SceneData* scene)
		:	commandIndex(-1)
		,	activeScene(scene)
		{ commands.reserve(UNDO_QUEUE_SIZE); };
	};

	typedef DAVA::Map<SceneData*, UndoQueue*> QUEUE_MAP;

    void ClearQueue(UndoQueue* queue);
    void ClearQueueTail(UndoQueue* queue);
	void ClearAllQueues();
	UndoQueue* GetQueueForScene(SceneData* scene);

	QUEUE_MAP queueMap;
};



#endif // __COMMANDS_MANAGER_H__