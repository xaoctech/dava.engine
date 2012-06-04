#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "Entity/Component.h"

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
    EntityFamilyType newFamilyType(oldFamilyType, addType);

    EntityFamily * oldFamily = GetFamilyByType(oldFamilyType); 
    EntityFamily * newFamily = GetFamilyByType(newFamilyType); 
        
    /*
        Если тип не равен 0, то есть если мы не удалили последний компонент. 
     */
    if (!newFamilyType.IsEmpty() && (newFamily == 0))
    {
        newFamily = new EntityFamily(this, newFamilyType);
            
        families[newFamilyType.GetBit()] = newFamily;
        
        // Require refactoring, because depends on intenrnal structure of FamilyType / ComponentType.
        uint64 bit = newFamilyType.GetBit();
        for (uint64 idx = 0; idx < 64; ++idx)
        {
            if (bit & (1 << idx))
            {
                Component * comp = Component::GetComponentByIndex(idx);
                familiesWithComponent.insert(std::pair<Component*, EntityFamily*>(comp, newFamily));
            }
        }
    }
    
    if (oldFamily && newFamily)
    {
        oldFamily->MoveToFamily(newFamily, entity);
    }else if (!newFamily)
    {
        oldFamily->DeleteEntity(entity);
    }else if (!oldFamily)
    {
        newFamily->NewEntity(entity);
    }
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


//template<class T>
//void EntityManager::GetLinkedTemplatePools(const char * dataName, List<TemplatePool<T> *> & poolList)
//{
//    std::pair<std::multimap<Component*, EntityFamily*>::iterator, std::multimap<Component*, EntityFamily*>::iterator> range;
//    range = familiesWithComponent.equal_range(component);
//    
//    TemplatePool<T> * currentPool = 0;
//    
//    for (std::multimap<Component*, EntityFamily*>::iterator it2 = range.first; it2 != range.second; ++it2)
//    {
//        EntityFamily * family = it2->second;
//        Pool * pool = family->GetPoolByDataName(dataName);
//        TemplatePool<T> * templatePool = dynamic_cast<TemplatePool<T>*>(pool); 
//        poolList.push_back(templatePool);
//
////      Not sure that can use references in pools, because they can potentially lie in several comps??? Need ot check
////        templatePool->next = currentPool;
////        currentPool = templatePool;
//    }
//    return currentPool;
//}
//
    
void EntityManager::RemoveComponent(Entity * entity, Component * component)
{
    
    
}
    
Pool * EntityManager::CreatePool(const char * dataName, int32 maxSize)
{
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
        newPool->next = prevPool;
        pools[dataName] = newPool;
    }
    return 0;
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
