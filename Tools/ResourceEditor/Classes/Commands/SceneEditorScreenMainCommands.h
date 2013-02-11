#ifndef __RESOURCEEDITORQT__SCENEEDITORSCREENMAINCOMMANDS__
#define __RESOURCEEDITORQT__SCENEEDITORSCREENMAINCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"

class DAVA::SceneNode;

class CommandCreateNodeSceneEditor: public Command
{
public:
	CommandCreateNodeSceneEditor(DAVA::SceneNode* node);
	virtual ~CommandCreateNodeSceneEditor();

protected:
	DAVA::SceneNode* node;

	virtual void Execute();
	virtual void Cancel();
};

#endif /* defined(__ResourceEditorQt__SceneEditorScreenMainCommands__) */
