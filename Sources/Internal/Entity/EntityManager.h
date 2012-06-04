#ifndef __DAVAENGINE_ENTITY_MANAGER_H__
#define __DAVAENGINE_ENTITY_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Entity/Entity.h"
#include "Entity/Pool.h"
#include "Entity/EntityFamily.h"

namespace DAVA 
{

class EntityManager : public Singleton<EntityManager>
{
public:
    Entity * CreateEntity();
	
    void AddComponent(Entity * entity, Component * component);
    void RemoveComponent(Entity * entity, Component * component);
	
    EntityFamily * GetFamily(Component * c0, ...);
    
    template<class T>
    void GetLinkedTemplatePoolsForComponent(Component * component, const char * dataName, List<TemplatePool<T> *> & poolList);
    
//    void Flush();
//
//	void EntityChanged(Entity * entity);
//
//	void Update();

private:
    // ?? 
    //
    //  e1, e2, e3, e4, e5
    //  
    //  
    //  
    //  
    
    
    Map<uint64, EntityFamily*> families;
    std::multimap<Component*, EntityFamily*> familiesWithComponent; // all families with given component
    
//	Set<Entity*> changedEntities;
//
//	void FlushEntity(Entity * entity);
//	void RemoveFromMap(Entity * entity );
//	void FlushFamily(EntityFamily * family);
};

};

#endif // __DAVAENGINE_ENTITY_MANAGER_H__
