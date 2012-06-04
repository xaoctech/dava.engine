#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Base/BaseTypes.h"
#include "Entity/ComponentTypes.h"

namespace DAVA
{

class Pool;
class Entity;
class Component;
class EntityManager;    

class EntityFamily
{
public:
	EntityFamily(EntityManager * manager, EntityFamilyType family);
    ~EntityFamily();
    
    void NewEntity(Entity * entity);
    void DeleteEntity(Entity * entity);
	void MoveToFamily(EntityFamily * newFamily, Entity * entity);
    
    Pool * GetPoolByDataName(const char * dataName);

    uint32 GetSize() { return currentSize; };
    template<class T>
    T * GetPtr(const char * name); 
    
private:
    EntityManager * manager;
    EntityFamilyType family;
    Vector<Entity*> entities;
	Vector<Pool*> pools;
    Map<const char *, Pool*> poolByDataName;
    uint32 currentSize;
    uint32 maxSize;
};

template<class T>
T * EntityFamily::GetPtr(const char * dataName)
{
    Pool * pool = GetPoolByDataName(dataName);
    DVASSERT(pool);

    // TODO: replace to reinterpret cast in release.
    TemplatePool<T> * tPool = dynamic_cast<TemplatePool<T>*>(pool);
    
    return tPool->GetHead();
}

    
};

#endif //__DAVAENGINE_ENTITY_FAMILY_H__
