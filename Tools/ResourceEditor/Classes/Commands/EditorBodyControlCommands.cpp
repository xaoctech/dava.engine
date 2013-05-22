/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "EditorBodyControlCommands.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "CommandsManager.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"

#include "Scene/SceneEditorProxy.h"
#include "Scene/System/CollisionSystem.h"

CommandEntityModification::CommandEntityModification(Command::eCommandType type, CommandList::eCommandId id)
:	Command(type, id)
{
}

DAVA::Set<DAVA::Entity*> CommandEntityModification::GetAffectedEntities()
{
	return entities;
}


CommandTransformObject::CommandTransformObject(DAVA::Entity* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_TRANSFORM_OBJECT)
{
	commandName = "Transform Object";

	undoTransform = originalTransform;
	redoTransform = finalTransform;

	entities.insert(node);
}

void CommandTransformObject::Execute()
{
	Entity* node = *entities.begin();

	if (node)
	{
		node->SetLocalTransform(redoTransform);
		UpdateCollision();
	}
	else
		SetState(STATE_INVALID);
}

void CommandTransformObject::Cancel()
{
	Entity* node = *entities.begin();

	if (node)
	{
		node->SetLocalTransform(undoTransform);
		UpdateCollision();
	}
}

void CommandTransformObject::UpdateCollision()
{
	Entity* node = *entities.begin();

	SceneEditorProxy *sep = dynamic_cast<SceneEditorProxy *>(node->GetScene());
	if(NULL != sep && NULL != sep->collisionSystem)
	{
		// make sure that worldtransform is up to date
		sep->transformSystem->Process();

		// update bullet object
		sep->collisionSystem->UpdateCollisionObject(node);
	}
}


CommandCloneObject::CommandCloneObject(DAVA::Entity* node, EditorBodyControl* bodyControl, btCollisionWorld* collisionWorld)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_CLONE_OBJECT)
,	collisionWorld(collisionWorld)
{
	commandName = "Clone Object";

	originalNode = node;
	this->bodyControl = bodyControl;
}

CommandCloneObject::~CommandCloneObject()
{
	if (!entities.empty())
	{
		Entity* clonedNode = *entities.begin();
		SafeRelease(clonedNode);
	}
}

void CommandCloneObject::Execute()
{
	if (originalNode && bodyControl)
	{
		Entity* clonedNode = 0;

		if (entities.empty())
		{
			clonedNode = originalNode->Clone();
			if (!clonedNode)
			{
				SetState(STATE_INVALID);
				return;
			}

			if (collisionWorld)
				UpdateCollision(clonedNode);

			entities.insert(clonedNode);
		}

		originalNode->GetParent()->AddNode(clonedNode);
		bodyControl->SelectNode(clonedNode);
	}
	else
		SetState(STATE_INVALID);
}

void CommandCloneObject::Cancel()
{
	Entity* clonedNode = GetClonedNode();

	if (originalNode && clonedNode)
	{
		clonedNode->GetParent()->RemoveNode(clonedNode);

		// rebuild scene graph after removing node
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		activeScene->RebuildSceneGraph();

		if (bodyControl)
			bodyControl->SelectNode(originalNode);
	}
}

void CommandCloneObject::UpdateCollision(DAVA::Entity *node)
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

Entity* CommandCloneObject::GetClonedNode()
{
	Entity* clonedNode = 0;
	if (!entities.empty())
	{
		clonedNode = *entities.begin();
	}

	return clonedNode;
}


CommandCloneAndTransform::CommandCloneAndTransform(DAVA::Entity* originalNode,
												   const DAVA::Matrix4& finalTransform,
												   EditorBodyControl* bodyControl,
												   btCollisionWorld* collisionWorld)
:	MultiCommand(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_CLONE_AND_TRANSFORM)
,	clonedNode(0)
,	cloneCmd(0)
,	transformCmd(0)
{
	commandName = "Clone Object";

	this->originalNode = originalNode;
	this->bodyControl = bodyControl;
	this->collisionWorld = collisionWorld;
	this->transform = finalTransform;
}

CommandCloneAndTransform::~CommandCloneAndTransform()
{
	SafeRelease(transformCmd);
	SafeRelease(cloneCmd);
}

void CommandCloneAndTransform::Execute()
{
	if (!cloneCmd)
	{
		cloneCmd = new CommandCloneObject(originalNode, bodyControl, collisionWorld);
	}
	ExecuteInternal(cloneCmd);

	if (GetInternalCommandState(cloneCmd) != STATE_VALID)
	{
		SetState(STATE_INVALID);
		return;
	}

	if (!transformCmd)
	{
		transformCmd = new CommandTransformObject(cloneCmd->GetClonedNode(), originalNode->GetLocalTransform(), transform);
		// Need to apply transform only once when creating command
		ExecuteInternal(transformCmd);

		if (GetInternalCommandState(transformCmd) != STATE_VALID)
		{
			SetState(STATE_INVALID);
			return;
		}
	}
}

void CommandCloneAndTransform::Cancel()
{
	// No need to undo transform command, removed node will appear at the same place where it was removed
	if (cloneCmd)
	{
		CancelInternal(cloneCmd);
	}
}


CommandPlaceOnLandscape::CommandPlaceOnLandscape(DAVA::Entity* node, EditorBodyControl* bodyControl)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_PLACE_ON_LANDSCAPE)
,	bodyControl(bodyControl)
{
	commandName = "Place On Landscape";

	redoTransform.Identity();

	if (node)
		undoTransform = node->GetLocalTransform();

	entities.insert(node);
}

void CommandPlaceOnLandscape::Execute()
{
	Entity* node = *entities.begin();

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
	Entity* node = *entities.begin();

	if (node)
	{
		node->SetLocalTransform(undoTransform);
	}
}


CommandRestoreOriginalTransform::CommandRestoreOriginalTransform(DAVA::Entity* node)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_RESTORE_ORIGINAL_TRANSFORM)
{
	commandName = "Restore Original Transform";

	if (node)
	{
		StoreCurrentTransform(node);
	}

	entities.insert(node);
}

void CommandRestoreOriginalTransform::Execute()
{
	Entity* node = *entities.begin();

	if (node)
	{
		node->RestoreOriginalTransforms();
	}
	else
		SetState(STATE_INVALID);
}

void CommandRestoreOriginalTransform::Cancel()
{
	Entity* node = *entities.begin();

	if (node)
	{
		RestoreTransform(node);
	}
}

void CommandRestoreOriginalTransform::StoreCurrentTransform(DAVA::Entity *node)
{
	if (node)
	{
		undoTransforms[node] = node->GetLocalTransform();

		for (int32 i = 0; i < node->GetChildrenCount(); ++i)
			StoreCurrentTransform(node->GetChild(i));
	}
}

void CommandRestoreOriginalTransform::RestoreTransform(DAVA::Entity *node)
{
	if (node)
	{
		Map<Entity*, Matrix4>::iterator it = undoTransforms.find(node);
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
