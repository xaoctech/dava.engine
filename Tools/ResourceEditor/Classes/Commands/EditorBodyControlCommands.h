#ifndef __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__
#define __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "BulletObject.h"

class EditorBodyControl;

class CommandTransformObject: public Command
{
public:
	CommandTransformObject(DAVA::Entity* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::Entity* node;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

class CommandCloneObject: public Command
{
public:
	CommandCloneObject(DAVA::Entity* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld);
	virtual ~CommandCloneObject();

	Entity* GetClonedNode();

protected:
	DAVA::Entity* originalNode;
	DAVA::Entity* clonedNode;
	EditorBodyControl* bodyControl;
	btCollisionWorld* collisionWorld;

	virtual void Execute();
	virtual void Cancel();

	void UpdateCollision(DAVA::Entity* node);
};

class CommandCloneAndTransform: public Command
{
public:
	CommandCloneAndTransform(DAVA::Entity* originalNode,
							 const DAVA::Matrix4& finalTransform,
							 EditorBodyControl* bodyControl,
							 btCollisionWorld* collisionWorld);
	virtual ~CommandCloneAndTransform();

protected:
	DAVA::Entity* originalNode;
	DAVA::Entity* clonedNode;
	EditorBodyControl* bodyControl;
	btCollisionWorld* collisionWorld;
	DAVA::Matrix4 transform;

	CommandCloneObject* cloneCmd;
	CommandTransformObject* transformCmd;

	virtual void Execute();
	virtual void Cancel();
};

class CommandPlaceOnLandscape: public Command
{
public:
	CommandPlaceOnLandscape(DAVA::Entity* node, EditorBodyControl* bodyControl);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::Entity* node;
	EditorBodyControl* bodyControl;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

class CommandRestoreOriginalTransform: public Command
{
public:
	CommandRestoreOriginalTransform(DAVA::Entity* node);

protected:
	DAVA::Map<DAVA::Entity*, DAVA::Matrix4> undoTransforms;
	DAVA::Entity* node;

	virtual void Execute();
	virtual void Cancel();

	void StoreCurrentTransform(DAVA::Entity* node);
	void RestoreTransform(DAVA::Entity* node);
};

#endif /* defined(__RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__) */
