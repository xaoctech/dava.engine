#include "Entity.h"
#include "EntityManager.h"

namespace DAVA 
{

Entity::Entity(EntityManager * _manager)
:	manager(_manager),
	family(0),
	indexInFamily(-1),
	changeState(0)
{
}

void Entity::AddComponent(Component * component)
{
    manager->AddComponent(this, component);

	changeState |= FAMILY_CHANGED;
}

void Entity::AddComponent(const char * component)
{
	//manager->AddComponent(this, component);
}

void Entity::RemoveComponent(Component * component)
{
	manager->RemoveComponent(this, component);

	changeState |= FAMILY_CHANGED;
}

//uint32 Entity::CalculateFamily()
//{
//	family = 0;
//    // TODO: Check how to write correct STL code, with size_type. Without 64 => 32 bit conversions.
//	uint32 componentsCount = (uint32)components.size();
//	for(uint32 i = 0; i < componentsCount; ++i)
//	{
//		family |= (1 << components[i]->type);
//	}
//
//	return family;
//}

void Entity::SetFamily(EntityFamilyType newFamily)
{
    family = newFamily;
}

const EntityFamilyType & Entity::GetFamily()
{
	return family;
}

void Entity::SetIndexInFamily(int32 _index)
{
	indexInFamily = _index;
}

int32 Entity::GetIndexInFamily()
{
	return indexInFamily;
}

};
