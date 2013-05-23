#ifndef __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__
#define __RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "BulletObject.h"

class EditorBodyControl;

class CommandEntityModification: public Command
{
public:
	CommandEntityModification(Command::eCommandType type, CommandList::eCommandId id);

protected:
	Set<Entity*> entities;

	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();
};

class CommandTransformObject: public CommandEntityModification
{
public:
	CommandTransformObject(DAVA::Entity* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform);

protected:
	DAVA::Matrix4 undoTransform;
	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();

	void UpdateCollision();
};

class CommandCloneObject: public CommandEntityModification
{
public:
	DAVA_DEPRECATED(CommandCloneObject(DAVA::Entity* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld));
	virtual ~CommandCloneObject();

	Entity* GetClonedNode();

protected:
	DAVA::Entity* originalNode;
	EditorBodyControl* bodyControl;
	btCollisionWorld* collisionWorld;

	virtual void Execute();
	virtual void Cancel();

	void UpdateCollision(DAVA::Entity* node);
};

class CommandCloneAndTransform: public MultiCommand
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

class CommandPlaceOnLandscape: public CommandEntityModification
{
public:
	CommandPlaceOnLandscape(DAVA::Entity* node, EditorBodyControl* bodyControl);

protected:
	DAVA::Matrix4 undoTransform;
	EditorBodyControl* bodyControl;

	DAVA::Matrix4 redoTransform;

	virtual void Execute();
	virtual void Cancel();
};

class CommandRestoreOriginalTransform: public CommandEntityModification
{
public:
	CommandRestoreOriginalTransform(DAVA::Entity* node);

protected:
	DAVA::Map<DAVA::Entity*, DAVA::Matrix4> undoTransforms;

	virtual void Execute();
	virtual void Cancel();

	void StoreCurrentTransform(DAVA::Entity* node);
	void RestoreTransform(DAVA::Entity* node);
};

#endif /* defined(__RESOURCEEDITORQT__EDITORBODYCONTROLCOMMANDS__) */
