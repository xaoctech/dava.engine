#include "Entity/EntityFamily.h"
#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"

namespace DAVA
{

EntityFamily::EntityFamily(EntityManager * _manager, EntityFamilyType _family)
{
    manager = _manager;
    family = _family;
    
    // Require refactoring, because depends on internal structure of FamilyType / ComponentType.
    Set<const char*> dataNamesForAllComponents;
    uint64 bit = family.GetBit();
    for (uint64 idx = 0; idx < 64; ++idx)
    {
        if (bit & ((int64)1 << idx))
        {
            Component * comp = Component::GetComponentByIndex(idx);
            dataNamesForAllComponents.insert(comp->GetDataNames().begin(), comp->GetDataNames().end());
        }
    }
    
    pools.reserve(dataNamesForAllComponents.size()); //this changes size of vector
    
    currentSize = 0;
	maxSize = 10;

    for (Set<const char*>::iterator it = dataNamesForAllComponents.begin(); it != dataNamesForAllComponents.end(); ++it)
    {
        Pool * newPool = manager->CreatePool(*it, maxSize);
		newPool->SetEntityFamily(this);
        pools.push_back(newPool);
        poolByDataName[*it] = newPool;
    }       
}
    
Pool * EntityFamily::GetPoolByDataName(const char * dataName)
{
    Map<const char *, Pool*>::iterator it = poolByDataName.find(dataName);
    if (it != poolByDataName.end())
    {
        return it->second;
    }
    return 0;
}
    
void EntityFamily::NewEntity(Entity * entity)
{
    //DVASSERT(entity->GetFamily() == EntityFamilyType(0));
    
    if (currentSize >= maxSize)
    {
        // Resize all pools
        for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
        {
            pools[poolIndex]->Resize(currentSize + 10);
        }
		maxSize += 10;
    }

    currentSize++;
	entities.push_back(entity);
    for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
    {
        pools[poolIndex]->length++;
    }
}
    
void EntityFamily::DeleteEntity(Entity * entity)
{
    // We can't only delete entity that in the family. If it's not something went wrong
    //DVASSERT(entity->GetFamily().GetBit() == this->family.GetBit());
    //DVASSERT(entity->indexInFamily >= 0);
    //DVASSERT(entity->indexInFamily < currentSize);
    
	int32 oldIndex = entity->GetIndexInFamily();
	int32 lastIndex = currentSize - 1;
    for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
    {
		if(pools[poolIndex]->length > 1)
		{
			pools[poolIndex]->MoveElement(lastIndex, oldIndex);
		}
        pools[poolIndex]->length--;
    }

	entities[oldIndex] = entities[lastIndex];
	entities.pop_back();

	currentSize--;
}
    
void EntityFamily::MoveFromFamily(EntityFamily * oldFamily, Entity * entity)
{
    int32 oldIndex = entity->GetIndexInFamily();
	int32 newIndex = currentSize;

	NewEntity(entity);
    
    for (Map<const char *, Pool*>::iterator currentPoolIt = poolByDataName.begin(); currentPoolIt != poolByDataName.end(); ++currentPoolIt)
    {
		Pool * oldPool = oldFamily->GetPoolByDataName(currentPoolIt->first);
        Pool * newPool = currentPoolIt->second;

		if(oldPool)
		{
			DVASSERT(typeid(*oldPool) == typeid(*newPool));
        
			oldPool->MoveElement(oldIndex, oldPool, newIndex);
		}
    }

	oldFamily->DeleteEntity(entity);
    
	//must stay
	entity->SetIndexInFamily(newIndex);
}


/*
void EntityFamily::Flush()
{
	// 

//	for (each moving entity in entities)
//	{
//		MoveToFamily(newFamily)
//	}
//
//	for (each deleted entity in entities)
//	{
//		Delete(entity);
//	}
//
//	if (currentSize + addedEntitiesSize > maxSize)
//		ReallocatePools(currentSize + addedEntitiesSize);
//
//	for (each added entity)
//	{
//		Add(entity);
//	}

	
	Set<String name>

	for each data in set
	{
		if 
		PoolSystem::Instance()->CreatePool(entities.size());
	}

}
 */

};
