#include "Entity/EntityFamily.h"
#include "Entity/Component.h"
#include "Entity/Entity.h"

namespace DAVA
{

EntityFamily::EntityFamily(EntityFamilyType _family)
{
    family = _family;
    
    uint32 poolCount = 0;
    
    // Require refactoring, because depends on intenrnal structure of FamilyType / ComponentType.
    uint64 bit = family.GetBit();
    for (uint64 idx = 0; idx < 64; ++idx)
    {
        if (bit & (1 << idx))
        {
            Component * comp = Component::GetComponentByIndex(idx);
            poolCount += comp->GetPoolCount();
        }
    }
    
    pools.resize(poolCount);
    
    currentSize = 0;
	maxSize = 15;

    // Require refactoring, because depends on intenrnal structure of FamilyType / ComponentType.
    for (uint64 idx = 0; idx < 64; ++idx)
    {
        if (bit & (1 << idx))
        {
            Component * comp = Component::GetComponentByIndex(idx);
            //comp->CreateCopyOfPools(pools, maxSize);
            Map<const char *, Pool *> & compPools = comp->GetNamedPools();
            for (Map<const char *, Pool *>::iterator it = compPools.begin(); it != compPools.end(); ++it)
            {
                Pool * newPool = it->second->CreateCopy(maxSize);
                pools.push_back(newPool);
                poolByComponentIndexDataName[std::pair<uint64, const char*>(idx, it->first)] = newPool;
            }
        }
    }
}
    
Pool * EntityFamily::GetPoolByComponentIndexDataName(uint64 index, const char * dataName)
{
    Map<std::pair<uint64, const char *>, Pool*>::iterator it = poolByComponentIndexDataName.find(std::pair<uint64, const char*>(index, dataName));
    if (it != poolByComponentIndexDataName.end())
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
