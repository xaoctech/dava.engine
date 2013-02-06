#include "EditorBodyControlCommands.h"
#include "EditorBodyControl.h"
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
