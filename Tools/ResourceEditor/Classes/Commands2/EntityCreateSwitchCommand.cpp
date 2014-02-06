/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "EntityCreateSwitchCommand.h"


EntityCreateSwitchCommand::EntityCreateSwitchCommand(DAVA::Entity* _entity, const DAVA::Vector<DAVA::Entity *> & fromEntities)
	: Command2(CMDID_ENTITY_CREATE_SWITCH, "Create Switch Entity")
	, entity(_entity)
	, sourceEntities(fromEntities)
{
	DVASSERT(entity);
	DVASSERT(NULL == GetSwitchComponent(entity));
	DVASSERT(NULL == GetRenderComponent(entity));
	DVASSERT(NULL == GetLodComponent(entity));
	
	entity->Retain();

	DAVA::uint32 count = (DAVA::uint32)sourceEntities.size();
	for(DAVA::uint32 i = 0; i < count; ++i)
	{
		sourceEntities[i]->Retain();
	}
}

EntityCreateSwitchCommand::~EntityCreateSwitchCommand()
{
	DAVA::uint32 count = (DAVA::uint32)sourceEntities.size();
	for(DAVA::uint32 i = 0; i < count; ++i)
	{
		sourceEntities[i]->Release();
	}
	sourceEntities.clear();

	entity->Release();
}

void EntityCreateSwitchCommand::Redo()
{
	DAVA::uint32 count = (DAVA::uint32)sourceEntities.size();

	DAVA::Vector<DAVA::Entity *> clonedEntities;
	clonedEntities.reserve(count);

	for(DAVA::uint32 i = 0; i < count; ++i)
	{
		clonedEntities.push_back(sourceEntities[i]->Clone());
	}

//////////////////////////////////////////////////////////////////////////
	DAVA::SwitchComponent *sw = new DAVA::SwitchComponent();
	entity->AddComponent(sw);

	DAVA::RenderObject *ro = new DAVA::Mesh();
	ro->SetAABBox(DAVA::AABBox3(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, 0)));    

	entity->AddComponent(new DAVA::RenderComponent(ro));
	ro->Release();

	DAVA::Set<DAVA::PolygonGroup*> bakedPolygonGroups;

	for(DAVA::uint32 i = 0; i < count; ++i)
	{
		DAVA::Entity * sourceEntity = clonedEntities[i];
		DAVA::RenderObject * sourceRenderObject = FindRenderObjectsRecursive(sourceEntity);
		DVASSERT(sourceRenderObject);

		//workaround for custom properties for crashed model
		if(1 == i) // crash model
		{
			DAVA::KeyedArchive *childProps = sourceEntity->GetCustomProperties();
			if(childProps->IsKeyExists("CollisionType"))
			{	
				DAVA::KeyedArchive *entityProps = entity->GetCustomProperties();
				entityProps->SetInt32("CollisionTypeCrashed", childProps->GetInt32("CollisionType", 0));
			}
		}
		//end of custom properties

		if(sourceRenderObject)
		{
			DAVA::TransformComponent * sourceTransform = GetTransformComponent(sourceEntity);
			if (sourceTransform->GetLocalTransform() != DAVA::Matrix4::IDENTITY)
			{
				DAVA::PolygonGroup * pg = sourceRenderObject->GetRenderBatchCount() > 0 ? sourceRenderObject->GetRenderBatch(0)->GetPolygonGroup() : 0;
				if(pg && bakedPolygonGroups.end() == bakedPolygonGroups.find(pg))
				{
					sourceRenderObject->BakeTransform(sourceTransform->GetLocalTransform());
					bakedPolygonGroups.insert(pg);
				}
			}

			DAVA::uint32 sourceSize = sourceRenderObject->GetRenderBatchCount();
			while(sourceSize)
			{
				DAVA::int32 lodIndex, switchIndex;
				DAVA::RenderBatch * sourceRenderBatch = sourceRenderObject->GetRenderBatch(0, lodIndex, switchIndex);
				sourceRenderBatch->Retain();
				sourceRenderObject->RemoveRenderBatch(sourceRenderBatch);
				ro->AddRenderBatch(sourceRenderBatch, lodIndex, i);
				sourceRenderBatch->Release();
				sourceSize--;
			}
		}

		DAVA::LodComponent * lc = GetLodComponent(sourceEntity);
		if((0 != lc) && (0 == GetLodComponent(entity)))
		{
			DAVA::LodComponent * newLod = (DAVA::LodComponent*)lc->Clone(entity);
			entity->AddComponent(newLod);
		}
	}

//////////////////////////////////////////////////////////////////////////


	for(DAVA::uint32 i = 0; i < count; ++i)
	{
		clonedEntities[i]->Release();
	}
	clonedEntities.clear();
}

void EntityCreateSwitchCommand::Undo()
{
	entity->RemoveComponent(DAVA::Component::SWITCH_COMPONENT);
	entity->RemoveComponent(DAVA::Component::RENDER_COMPONENT);

	if(GetLodComponent(entity))
		entity->RemoveComponent(DAVA::Component::LOD_COMPONENT);
}


DAVA::Entity* EntityCreateSwitchCommand::GetEntity() const
{
	return entity;
}


DAVA::RenderObject * EntityCreateSwitchCommand::FindRenderObjectsRecursive(DAVA::Entity * fromEntity) const
{
	DAVA::RenderObject * ro = GetRenderObject(fromEntity);
	if(ro && ro->GetType() == DAVA::RenderObject::TYPE_MESH)
	{
		return ro; 
	}

	DAVA::int32 size = fromEntity->GetChildrenCount();
	for(DAVA::int32 i = 0; i < size; ++i)
	{
		DAVA::Entity * child = fromEntity->GetChild(i);
		ro = FindRenderObjectsRecursive(child);
		if(ro)
			return ro;
	}

	return NULL;
}
