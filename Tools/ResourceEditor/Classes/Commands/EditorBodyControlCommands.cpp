#include "EditorBodyControlCommands.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/SceneEditorScreenMain.h"

CommandTransformObject::CommandTransformObject(DAVA::SceneNode* node, const DAVA::Matrix4& originalTransform, const DAVA::Matrix4& finalTransform)
:	Command(COMMAND_UNDO_REDO)
{
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


CommandCloneObject::CommandCloneObject(DAVA::SceneNode* node, EditorBodyControl* bodyControl)
:	Command(COMMAND_UNDO_REDO)
,	clonedNode(NULL)
{
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

CommandPlaceOnLandscape::CommandPlaceOnLandscape(DAVA::SceneNode* node, DAVA::LandscapeNode* landscape)
:	Command(COMMAND_UNDO_REDO)
{
	this->node = node;
	this->landscape = landscape;

	if (node)
		undoTransform = node->GetLocalTransform();
}

void CommandPlaceOnLandscape::Execute()
{
	if (node && landscape)
	{
		const Matrix4& worldTransform = node->GetWorldTransform();
		Vector3 p = Vector3(0, 0, 0) * worldTransform;
		
		Vector3 result;
		bool res = landscape->PlacePoint(p, result);
		if (res)
		{
			Vector3 offset = result - p;
			redoTransform.CreateTranslation(offset);
			redoTransform = node->GetLocalTransform() * redoTransform;
		}
		
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

		for (uint32 i = 0; i < node->GetChildrenCount(); ++i)
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

		for (uint32 i = 0; i < node->GetChildrenCount(); ++i)
		{
			RestoreTransform(node->GetChild(i));
		}
	}
}
