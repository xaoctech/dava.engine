#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Base/BaseTypes.h"


namespace DAVA
{

class Pool;
class Entity;

class EntityFamily
{
public:
	EntityFamily(uint32 family);
	void Flush();

	void MoveToFamily(EntityFamily * newFamily, Entity * entity);

	Vector<Entity*> entities;
private:
	int32 currentSize;
	int32 maxSize;
	Vector<Pool*> pools;
	uint32 family;
};

};

#endif //__DAVAENGINE_ENTITY_FAMILY_H__
