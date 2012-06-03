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
	
	void Flush();

	void EntityChanged(Entity * entity);

	void Update();

private:
	Set<Entity*> changedEntities;
	Map<int32, EntityFamily> families;

	void FlushEntity(Entity * entity);
	void RemoveFromMap( Entity * entity );
	void FlushFamily(EntityFamily * family);
};

};

#endif // __DAVAENGINE_ENTITY_MANAGER_H__
