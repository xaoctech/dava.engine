#ifndef __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__
#define __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "BulletObject.h"

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
	CommandCloneObject(DAVA::SceneNode* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld);
	virtual ~CommandCloneObject();

protected:
	DAVA::SceneNode* originalNode;
	DAVA::SceneNode* clonedNode;
	EditorBodyControl* bodyControl;
	btCollisionWorld* collisionWorld;

	virtual void Execute();
	virtual void Cancel();

	void UpdateCollision(DAVA::SceneNode* node);
};

class CommandPlaceOnLandscape: public Command
{
public:
	CommandPlaceOnLandscape(DAVA::SceneNode* node, EditorBodyControl* bodyControl);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::SceneNode* node;
	EditorBodyControl* bodyControl;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

class CommandRestoreOriginalTransform: public Command
{
public:
	CommandRestoreOriginalTransform(DAVA::SceneNode* node);

protected:
	DAVA::Map<DAVA::SceneNode*, DAVA::Matrix4> undoTransforms;
	DAVA::SceneNode* node;

	virtual void Execute();
	virtual void Cancel();

	void StoreCurrentTransform(DAVA::SceneNode* node);
	void RestoreTransform(DAVA::SceneNode* node);
};

#endif /* defined(__RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__) */
