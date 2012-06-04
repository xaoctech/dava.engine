#include "EntityManager.h"
#include "Component.h"

namespace DAVA 
{

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

    EntityFamily * oldFamily = 0; 
    EntityFamily * newFamily = 0; 
    
    Map<EntityFamilyType, EntityFamily*>::iterator oldFamilyIterator = families.find(oldFamilyType);
    if (oldFamilyIterator != families.end())
    {
        oldFamily = oldFamilyIterator->second;
    }
    
    Map<EntityFamilyType, EntityFamily*>::iterator newFamilyIterator = families.find(newFamilyType);
    if (newFamilyIterator != families.end())
    {
        newFamily = newFamilyIterator->second;
    }
    
    /*
        Если тип не равен 0, то есть если мы не удалили последний компонент. 
     */
    if (!newFamilyType.IsEmpty() && (newFamily == 0))
    {
        newFamily = new EntityFamily(newFamilyType);
            
        families[newFamilyType] = newFamily;
        
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
    
template<class T>
TemplatePool<T> * EntityManager::GetLinkedTemplatePoolsForComponent(Component * component, const char * dataName)
{
    std::pair<std::multimap<Component*, EntityFamily*>::iterator, std::multimap<Component*, EntityFamily*>::iterator> range;
    range = familiesWithComponent.equal_range(component);
    
    TemplatePool<T> * currentPool = 0;
    
    for (std::multimap<Component*, EntityFamily*>::iterator it2 = range.first; it2 != range.second; ++it2)
    {
        EntityFamily * family = it2->second;
        Pool * pool = family->GetPoolByComponentIndexDataName(component->GetType().GetIndex(), dataName);
        TemplatePool<T> * templatePool = dynamic_cast<TemplatePool<T>*>(pool); 
        templatePool->next = currentPool;
        currentPool = templatePool;
    }
    return currentPool;
}

    
void EntityManager::RemoveComponent(Entity * entity, Component * component)
{
    
    
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
