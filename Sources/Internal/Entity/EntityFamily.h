#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Base/BaseTypes.h"
#include "Entity/ComponentType.h"

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

private:
    EntityFamilyType family;
    Vector<Entity*> entities;
	Vector<Pool*> pools;
    Map<std::pair<uint64, const char *>, Pool*> poolByComponentIndexDataName;
    uint32 currentSize;
    uint32 maxSize;
};

};

#endif //__DAVAENGINE_ENTITY_FAMILY_H__
