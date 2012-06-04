#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Base/BaseTypes.h"
#include "Entity/ComponentTypes.h"
#include "Entity/Component.h"

namespace DAVA
{

class Pool;
class Entity;

class EntityFamily
{
public:
	EntityFamily(EntityFamilyType family);
    ~EntityFamily();
    
    void NewEntity(Entity * entity);
    void DeleteEntity(Entity * entity);
	void MoveToFamily(EntityFamily * newFamily, Entity * entity);
    
    Pool * GetPoolByComponentIndexDataName(uint64 index, const char * dataName);

    uint32 GetSize() { return currentSize; };
    template<class T>
    T * GetPtr(Component * component, const char * name); 
    
private:
    EntityFamilyType family;
    Vector<Entity*> entities;
	Vector<Pool*> pools;
    Map<std::pair<uint64, const char *>, Pool*> poolByComponentIndexDataName;
    uint32 currentSize;
    uint32 maxSize;
};

template<class T>
T * EntityFamily::GetPtr(Component * component, const char * dataName)
{
    Pool * pool = GetPoolByComponentIndexDataName(component->GetType().GetIndex(), dataName);
    
    // TODO: replace to reinterpret cast in release.
    TemplatePool<T> * tPool = dynamic_cast<TemplatePool<T>*>(pool);
    
    return tPool->GetHead();
}

    
};

#endif //__DAVAENGINE_ENTITY_FAMILY_H__
