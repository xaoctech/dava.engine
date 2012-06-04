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
	maxSize = 15;

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
            pools[poolIndex]->Resize(currentSize + 15);
        }
    }
    entity->SetIndexInFamily((int32)currentSize);
    currentSize++;
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
    
    for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
    {
        pools[poolIndex]->MoveElement(currentSize - 1, entity->GetIndexInFamily());
        pools[poolIndex]->length--;
    }
    currentSize--;
    entity->SetIndexInFamily(-1);
    entity->SetFamily(EntityFamilyType(0)); // Empty family
}
    
void EntityFamily::MoveToFamily(EntityFamily * newFamily, Entity * entity)
{
    // We should move all data from one pools to other pools.
    
    if (currentSize >= maxSize)
    {
        // Resize all pools
        for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
        {
            pools[poolIndex]->Resize(currentSize + 15);
        }
    }
    int32 oldIndex = entity->GetIndexInFamily();
    
    for (Map<const char *, Pool*>::iterator currentPoolIt = poolByDataName.begin(); currentPoolIt != poolByDataName.end(); ++currentPoolIt)
    {
        Pool * newFamilyPool = newFamily->GetPoolByDataName(currentPoolIt->first);
        DVASSERT(typeid(*newFamilyPool) == typeid(*newFamilyPool));
        
        newFamilyPool->MoveElement(oldIndex, (uint32)currentSize);
    }
    
    entity->SetIndexInFamily((int32)currentSize);
    currentSize++;
    for (uint32 poolIndex = 0; poolIndex < pools.size(); ++poolIndex)
    {
        pools[poolIndex]->length++;
    }
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
