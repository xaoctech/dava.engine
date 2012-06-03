#include "Entity.h"
#include "EntityManager.h"

namespace DAVA 
{

Entity::Entity(EntityManager * _manager)
:	manager(_manager),
	changeState(CREATED),
	family(0),
	indexInFamily(-1)
{
	manager->EntityChanged(this);
}

void Entity::AddComponent(Component * component)
{
    components.push_back(component);
	changeState |= COMPONENT_ADDED;
	manager->EntityChanged(this);
}

uint32 Entity::CalculateFamily()
{
	family = 0;
	int32 componentsCount = components.size();
	for(int32 i = 0; i < componentsCount; ++i)
	{
		family |= (1 << components[i]->type);
	}

	return family;
}

uint32 Entity::GetFamily()
{
	return family;
}

int32 Entity::GetChangeState()
{
	return changeState;
}

void Entity::SetIndex(int32 _index)
{
	indexInFamily = _index;
}

int32 Entity::GetIndex()
{
	return indexInFamily;
}



};
