#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Math/AABBox3.h"

namespace DAVA 
{

    
Map<const char *, Pool *> EntityManager::poolAllocators;

Entity * EntityManager::CreateEntity()
{
	Entity * entity = new Entity(this);
	return entity;
}
    
void EntityManager::AddComponent(Entity * entity, Component * component)
{
    const ComponentType & addType = component->GetType();
    EntityFamilyType oldFamilyType = entity->GetFamily();
    EntityFamilyType newFamilyType = EntityFamilyType::AddComponentType(oldFamilyType, addType);

	ProcessAddRemoveComponent(entity, oldFamilyType, newFamilyType);
}

void EntityManager::AddComponent(Entity * entity, const char * componentName)
{
	Map<const char *, Component * >::iterator it = Component::cache.find(componentName);
	if(it != Component::cache.end())
	{
		AddComponent(entity, it->second);
	}
}

void EntityManager::RemoveComponent(Entity * entity, Component * component)
{
	const ComponentType & addType = component->GetType();
	EntityFamilyType oldFamilyType = entity->GetFamily();
	EntityFamilyType newFamilyType = EntityFamilyType::RemoveComponentType(oldFamilyType, addType);

	ProcessAddRemoveComponent(entity, oldFamilyType, newFamilyType);
}

void EntityManager::ProcessAddRemoveComponent(Entity * entity, const EntityFamilyType & oldFamilyType, const EntityFamilyType & newFamilyType)
{
    EntityFamily * oldFamily = GetFamilyByType(oldFamilyType); 
    EntityFamily * newFamily = GetFamilyByType(newFamilyType); 
        
    /*
        Если тип не равен 0, то есть если мы не удалили последний компонент. 
     */
    if (!newFamilyType.IsEmpty() && (newFamily == 0))
    {
        newFamily = new EntityFamily(this, newFamilyType);
            
        families[newFamilyType.GetBit()] = newFamily;
        
        // Require refactoring, because depends on internal structure of FamilyType / ComponentType.
        uint64 bit = newFamilyType.GetBit();
        for (uint64 idx = 0; idx < 64; ++idx)
        {
            if (bit & ((int64)1 << idx))
            {
                Component * comp = Component::GetComponentByIndex(idx);
                familiesWithComponent.insert(std::pair<Component*, EntityFamily*>(comp, newFamily));
            }
        }
    }
    
    if (oldFamily && newFamily)
    {
		newFamily->MoveFromFamily(oldFamily, entity);
    }
	else if (!newFamily)
    {
        oldFamily->DeleteEntity(entity);
    }
	else if (!oldFamily)
    {
        newFamily->NewEntity(entity);
    }

	entity->SetFamily(newFamilyType);
	entity->SetIndexInFamily(newFamily->GetSize()-1);
}

EntityFamily * EntityManager::GetFamilyByType(const EntityFamilyType & familyType)
{
    Map<uint64, EntityFamily*>::iterator familyIterator = families.find(familyType.GetBit());
    if (familyIterator != families.end())
    {
        return familyIterator->second;
    }   
    return 0;
}
    
EntityFamily * EntityManager::GetFamily(Component * c0, ...)
{
    va_list list;
    
    va_start(list, c0);
    
    uint64 bit = c0->GetType().GetBit();
    while(1)
    {
        Component * cNext = va_arg(list, Component*);
        if (!cNext)break;
        bit |= cNext->GetType().GetBit();
    }
    va_end(list);

    Map<uint64, EntityFamily*>::iterator familyIterator = families.find(bit);
    if (familyIterator != families.end())
    {
        return familyIterator->second;
    }
    return 0;
}
    
Pool * EntityManager::CreatePool(const char * dataName, int32 maxSize)
{
	Pool * pool = 0;

    Map<const char *, Pool *>::iterator poolsIt = poolAllocators.find(dataName);
    if (poolsIt != poolAllocators.end())
    {
        Pool * newPool = poolsIt->second->CreateCopy(maxSize);
        
        Pool * prevPool = 0;
        Map<const char *, Pool*>::iterator find = pools.find(dataName);
        if(pools.end() != find)
        {
            prevPool = find->second;
        }
        newPool->SetNext(prevPool);
        pools[dataName] = newPool;
		pool = newPool;
    }
	
	return pool;
}

void EntityManager::Dump()
{
	Logger::Info("============================");
	Logger::Info("EntityManager dump");
	Logger::Info("============================");
	Logger::Info("Pools:");
	Logger::Info("============================");
	
	Map<const char *, Pool *>::iterator poolIterator;
	for(poolIterator = pools.begin(); poolIterator != pools.end(); ++poolIterator)
	{
		const char * poolName = poolIterator->first;
		Pool * pool = poolIterator->second;
		Logger::Info("Pool \"%s\" of type %s", poolName, typeid(*pool).name());
		Logger::Info("----------------------------");
		
		while(pool)
		{
			int32 count = pool->GetCount();
			int32 maxCount = pool->GetMaxCount();
			Logger::Info("    subpool of family %lld (%d elements, %d maxCount, %d bytes)", pool->GetEntityFamily()->family.GetBit(), count, maxCount, maxCount*pool->typeSizeof);
			for(int32 i = 0; i < count; ++i)
			{
				pool->DumpElement(i);
			}

			pool = pool->GetNext();
			Logger::Info("    ----------------------------");
		}
	}
}

//void EntityManager::EntityChanged(Entity * entity)
//{
//	changedEntities.insert(entity);
//}
//
//void EntityManager::Update()
//{
//
//}
/*
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

		uint32 entityType = entity->CalculateFamily();
		families[entityType].entities.push_back(entity);
		entity->SetIndex((uint32)families[entityType].entities.size() - 1);
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
}*/





};
