#ifndef __RESOURCEEDITORQT__SCENEEDITORSCREENMAINCOMMANDS__
#define __RESOURCEEDITORQT__SCENEEDITORSCREENMAINCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"

class DAVA::Entity;

class CommandCreateNodeSceneEditor: public Command
{
public:
	CommandCreateNodeSceneEditor(DAVA::Entity* node);
	virtual ~CommandCreateNodeSceneEditor();

protected:
	DAVA::Entity* node;

	virtual void Execute();
	virtual void Cancel();
	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();
};

#endif /* defined(__ResourceEditorQt__SceneEditorScreenMainCommands__) */
