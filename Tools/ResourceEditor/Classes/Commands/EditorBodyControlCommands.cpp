#include "EditorBodyControlCommands.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneEditorScreenMain.h"

CommandTransformObject::CommandTransformObject(DAVA::SceneNode* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform)
:	Command(COMMAND_UNDO_REDO)
{
	commandName = "Transform Object";

	undoTransform = originalTransform;
	redoTransform = finalTransform;
	this->node = node;
}

void CommandTransformObject::Execute()
{
	if (node)
	{
		node->SetLocalTransform(redoTransform);
	}
	else
		SetState(STATE_INVALID);
}

void CommandTransformObject::Cancel()
{
	if (node)
	{
		node->SetLocalTransform(undoTransform);
	}
}


CommandCloneObject::CommandCloneObject(DAVA::SceneNode* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld)
:	Command(COMMAND_UNDO_REDO)
,	clonedNode(NULL)
,	collisionWorld(collisionWorld)
{
	commandName = "Clone Object";

	originalNode = node;
	this->bodyControl = bodyControl;
}

CommandCloneObject::~CommandCloneObject()
{
	SafeRelease(clonedNode);
}

void CommandCloneObject::Execute()
{
	if (originalNode && bodyControl)
	{
		if (!clonedNode)
		{
			clonedNode = SafeRetain(originalNode->Clone());

			if (collisionWorld)
				UpdateCollision(clonedNode);
		}

		originalNode->GetParent()->AddNode(clonedNode);
		bodyControl->SelectNode(clonedNode);
	}
	else
		SetState(STATE_INVALID);
}

void CommandCloneObject::Cancel()
{
	if (originalNode && clonedNode)
	{
		bodyControl->RemoveNode(clonedNode);

		if (bodyControl)
			bodyControl->SelectNode(originalNode);
	}
}

void CommandCloneObject::UpdateCollision(DAVA::SceneNode *node)
{
	DVASSERT(node && collisionWorld);
	
	BulletComponent* bc = dynamic_cast<BulletComponent*>(node->GetComponent(Component::BULLET_COMPONENT));
	if (bc && !bc->GetBulletObject())
	{
		bc->SetBulletObject(ScopedPtr<BulletObject>(new BulletObject(node->GetScene(),
																	 collisionWorld,
																	 node,
																	 node->GetWorldTransform())));
	}
	
	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		UpdateCollision(node->GetChild(i));
	}
}


CommandPlaceOnLandscape::CommandPlaceOnLandscape(DAVA::SceneNode* node, EditorBodyControl* bodyControl)
:	Command(COMMAND_UNDO_REDO)
,	node(node)
,	bodyControl(bodyControl)
{
	commandName = "Place On Landscape";

	redoTransform.Identity();

	if (node)
		undoTransform = node->GetLocalTransform();
}

void CommandPlaceOnLandscape::Execute()
{
	if (node && bodyControl)
	{
		redoTransform = node->GetLocalTransform() * bodyControl->GetLandscapeOffset(node->GetWorldTransform());
		node->SetLocalTransform(redoTransform);
	}
	else
		SetState(STATE_INVALID);
}

void CommandPlaceOnLandscape::Cancel()
{
	if (node)
	{
		node->SetLocalTransform(undoTransform);
	}
}


CommandRestoreOriginalTransform::CommandRestoreOriginalTransform(DAVA::SceneNode* node)
:	Command(COMMAND_UNDO_REDO)
,	node(node)
{
	commandName = "Restore Original Transform";

	if (node)
	{
		StoreCurrentTransform(node);
	}
}

void CommandRestoreOriginalTransform::Execute()
{
	if (node)
	{
		node->RestoreOriginalTransforms();
	}
	else
		SetState(STATE_INVALID);
}

void CommandRestoreOriginalTransform::Cancel()
{
	if (node)
	{
		RestoreTransform(node);
	}
}

void CommandRestoreOriginalTransform::StoreCurrentTransform(DAVA::SceneNode *node)
{
	if (node)
	{
		undoTransforms[node] = node->GetLocalTransform();

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
			StoreCurrentTransform(node->GetChild(i));
	}
}

void CommandRestoreOriginalTransform::RestoreTransform(DAVA::SceneNode *node)
{
	if (node)
	{
		Map<SceneNode*, Matrix4>::iterator it = undoTransforms.find(node);
		if (it != undoTransforms.end())
		{
			node->SetLocalTransform((*it).second);
		}

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
		{
			RestoreTransform(node->GetChild(i));
		}
	}
}
