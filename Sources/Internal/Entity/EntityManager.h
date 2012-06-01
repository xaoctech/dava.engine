#ifndef __DAVAENGINE_ENTITY_MANAGER_H__
#define __DAVAENGINE_ENTITY_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"
#include "Entity/Entity.h"
#include "Entity/Pool.h"

namespace DAVA 
{

class EntityManager : public StaticSingleton<EntityManager>
{
public:
    Entity * CreateEntity();

private:
	Vector<Entity*> entities;
};

};

#endif // __DAVAENGINE_ENTITY_MANAGER_H__