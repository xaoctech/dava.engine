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
    
    // Require refactoring, because depends on intenrnal structure of FamilyType / ComponentType.
    
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
    
    pools.resize(dataNamesForAllComponents.size());
    
    currentSize = 0;
	maxSize = 15;

    Map<const char *, Pool *> & compPools = EntityManager::GetPoolAllocators();

    for (Set<const char*>::iterator it = dataNamesForAllComponents.begin(); it != dataNamesForAllComponents.end(); ++it)
    {
        Pool * newPool = manager->CreatePool(*it, maxSize);
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
}
    
void EntityFamily::DeleteEntity(Entity * entity)
{
    
    
    
}
    
void EntityFamily::MoveToFamily(EntityFamily * newFamily, Entity * entity)
{
    
    
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
