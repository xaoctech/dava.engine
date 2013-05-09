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

CommandGroupEntitiesForMultiselect::CommandGroupEntitiesForMultiselect(const EntityGroup* entities)
:	CommandEntityModification(COMMAND_UNDO_REDO, CommandList::ID_COMMAND_UNITE_ENTITIES_FOR_MULTISELECT)
{
	commandName = "Unite entities for multiselect";
	this->entitiesToGroup = (*entities);
	this->resultEntity = NULL;
	Entity* en = entitiesToGroup.GetEntity(0);
	sep = NULL;
	if(NULL != en)
	{
		sep = dynamic_cast<SceneEditorProxy *>(en->GetScene());
	}
}

void CommandGroupEntitiesForMultiselect::Execute()
{
	if(entitiesToGroup.Size() < 2)
	{
		return;
	}

	Vector3 originalModifEntitiesCenter = entitiesToGroup.GetCommonBbox().GetCenter();
							
	Entity* solidEntityToAdd = entitiesToGroup.GetEntity(0);
	// search of  proper parent entity(with castom property isSolid)
	while (NULL != solidEntityToAdd)
	{
		KeyedArchive *customProperties = solidEntityToAdd->GetCustomProperties();
        if(customProperties && customProperties->IsKeyExists(String(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME)))
		{
			break;
		}

		solidEntityToAdd = solidEntityToAdd->GetParent();
	};
	Entity* parent = solidEntityToAdd->GetParent();
	Entity* complexEntity = new Entity();
	complexEntity->SetName("textComplexEntity");
	complexEntity->SetDebugFlags(DebugRenderComponent::DEBUG_DRAW_ALL, true);

	parent->AddNode(complexEntity);
	MoveEntity(complexEntity, originalModifEntitiesCenter);

	for(size_t i = 0; i < entitiesToGroup.Size(); ++i)
	{
		Entity *en = entitiesToGroup.GetEntity(i);
		solidEntityToAdd = en;
		// search of  proper parent entity(with castom property isSolid)
		while (NULL != solidEntityToAdd)
		{
			KeyedArchive *customProperties = solidEntityToAdd->GetCustomProperties();
		    if(customProperties && customProperties->IsKeyExists(String(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME)))
			{
				break;
			}

			solidEntityToAdd = solidEntityToAdd->GetParent();
		};
		if(NULL != en)
		{
			DAVA::AABBox3 ret;
			sep->collisionSystem->GetBoundingBox(en).GetTransformedBox(en->GetWorldTransform(), ret);
			originalMatrixes[en] = std::make_pair<Matrix4, Matrix4>(en->GetLocalTransform(), en->GetWorldTransform());
			originalChildParentRelations[solidEntityToAdd] = solidEntityToAdd->GetParent();
			complexEntity->AddNode(solidEntityToAdd);
		}
	}
	
	LodSystem::MergeChildLods(complexEntity);

	for(size_t i = 0; i < entitiesToGroup.Size(); ++i)
	{
		Entity* en = entitiesToGroup.GetEntity(i);
		UpdateTransformMatrixes(en, originalMatrixes[en].first, originalMatrixes[en].second);
	}

	resultEntity = complexEntity;
}

void CommandGroupEntitiesForMultiselect::Cancel()
{
	for(Map<Entity*, Entity*>::iterator itParent = originalChildParentRelations.begin();
		itParent != originalChildParentRelations.end(); ++itParent)
	{
		Entity* parent = (*itParent).second;
		if(NULL != parent)
		{
			parent->AddNode((*itParent).first);
		}
	}

	for(Map<Entity*, std::pair<Matrix4, Matrix4>>::iterator itPosition = originalMatrixes.begin();
		itPosition != originalMatrixes.end(); ++itPosition)
	{
		Entity* entity = (*itPosition).first;

		if(NULL != entity)
		{
			UpdateTransformMatrixes(entity, itPosition->second.first, itPosition->second.second);
		}
	}
	resultEntity->GetParent()->RemoveNode(resultEntity);
	SafeRelease(resultEntity);
}

void CommandGroupEntitiesForMultiselect::UpdateTransformMatrixes(Entity* entity, const Matrix4& localMatrix, Matrix4& worldMatrix)
{
	if(NULL == entity)
	{
		return;
	}
	Matrix4 originalTransform = entity->GetLocalTransform();
	entity->SetLocalTransform(localMatrix);
	
	DAVA::Matrix4 newTransform = worldMatrix;

	// to move the entity directly to absolute coordinates all paretn matrixes must be multiplied
	// and be taken into consideration beacause they are procesed  in TransformSystem (HierahicFindUpdatableTransform)
	if(entity->GetParent() != NULL)
	{
		Entity* parent = entity->GetParent();
		//Matrix4 parentMatrix = entity->GetParent()->GetWorldTransform();
		Matrix4 parentMatrix = Matrix4::IDENTITY;

		//calculate paretn matrixes through entire parent tree
		while (parent && !(parent->GetLocalTransform() == Matrix4::IDENTITY && parent->GetWorldTransform() == Matrix4::IDENTITY))
		{
			Matrix4 tempMatrix = parentMatrix * parent->GetLocalTransform();
			parentMatrix = tempMatrix;
			parent = parent->GetParent();
		};
		
		// newTransform should be devided by parentMatrix, because it would be multiplied in HierahicFindUpdatableTransform
		// matrix operation : A / B = ( B ^ (-1) ) * A
		Matrix4 inversedParetnMatrix;//(B ^ (-1))

		bool canBeInversed = parentMatrix.GetInverse(inversedParetnMatrix);
		if(!canBeInversed)
		{
			return;
		}

		Matrix4 absoluteNewTransform =  newTransform * inversedParetnMatrix ; //( B ^ (-1) ) * A
		newTransform = absoluteNewTransform;
	}

	CommandsManager::Instance()->ExecuteAndRelease(new CommandTransformObject(entity, originalTransform, newTransform));
}

void CommandGroupEntitiesForMultiselect::MoveEntity(Entity* entity, Vector3& destPoint)
{
	if(NULL == sep || NULL == entity)
	{
		return;
	}
	DAVA::AABBox3 currentItemBB;
	sep->collisionSystem->GetBoundingBox(entity).GetTransformedBox(entity->GetWorldTransform(), currentItemBB);

	Vector3 centrOfEntity = currentItemBB.GetCenter();
	Matrix4 originalTransform = entity->GetLocalTransform();
	DAVA::Vector3 moveOffset = destPoint - centrOfEntity;
	DAVA::Matrix4 moveModification;
	moveModification.CreateTranslation(moveOffset);
	
	DAVA::Matrix4 newTransform = entity->GetWorldTransform() * moveModification;

	UpdateTransformMatrixes(entity, Matrix4::IDENTITY, newTransform);
}

Entity* CommandGroupEntitiesForMultiselect::GetResultEntity()
{
	return resultEntity;
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
