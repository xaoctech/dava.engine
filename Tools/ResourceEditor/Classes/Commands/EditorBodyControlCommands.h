#ifndef __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__
#define __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"

class EditorBodyControl;

class CommandTransformObject: public Command
{
public:
	CommandTransformObject(DAVA::SceneNode* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::SceneNode* node;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

class CommandCloneObject: public Command
{
public:
	CommandCloneObject(DAVA::SceneNode* node, EditorBodyControl* bodyControl);
	virtual ~CommandCloneObject();

protected:
	DAVA::SceneNode* originalNode;
	DAVA::SceneNode* clonedNode;
	EditorBodyControl* bodyControl;

	virtual void Execute();
	virtual void Cancel();
};

class CommandPlaceOnLandscape: public Command
{
public:
	CommandPlaceOnLandscape(DAVA::SceneNode* node, DAVA::LandscapeNode* landscape);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::SceneNode* node;
	DAVA::LandscapeNode* landscape;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

#endif /* defined(__RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__) */
