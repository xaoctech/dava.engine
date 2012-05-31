#include "EntityManager.h"

Entity * EntityManager::CreateEntity()
{
	Entity * entity = new Entity();
	entities.push_back(entity);

	return entity;
}
