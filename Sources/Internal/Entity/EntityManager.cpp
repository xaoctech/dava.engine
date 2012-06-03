#include "EntityManager.h"
#include "Component.h"

namespace DAVA 
{

Entity * EntityManager::CreateEntity()
{
	Entity * entity = new Entity(this);
	return entity;
}

void EntityManager::EntityChanged(Entity * entity)
{
	changedEntities.insert(entity);
}

void EntityManager::Update()
{

}

void EntityManager::Flush()
{
	Set<Entity*>::iterator entityIter = changedEntities.begin();
	Set<Entity*>::iterator entitiesEnd = changedEntities.end();
	for(; entityIter != entitiesEnd; ++entityIter)
	{
		FlushEntity(*entityIter);
	}

	Map<int32, EntityFamily>::iterator familyIter = families.begin();
	Map<int32, EntityFamily>::iterator familiesEnd = families.end();
	for(; familyIter != familiesEnd; ++familyIter)
	{
		FlushFamily(&(familyIter->second));
	}
}

void EntityManager::FlushEntity(Entity * entity)
{
	if(entity->GetChangeState() & Entity::CREATED ||
		entity->GetChangeState() & Entity::COMPONENT_ADDED)
	{
		if(entity->GetIndex() != -1)
		{
			RemoveFromMap(entity);
		}

		int32 entityType = entity->CalculateFamily();
		families[entityType].entities.push_back(entity);
		entity->SetIndex(families[entityType].entities.size()-1);
	}

}

void EntityManager::RemoveFromMap(Entity * entity)
{
	if(entity->GetIndex() != -1)
	{
		uint32 oldType = entity->GetFamily();
		//entities[oldType][entity->GetIndex()]
		//TODO: 
	}
}

void EntityManager::FlushFamily(EntityFamily * family)
{
	family->Flush();
}

};
