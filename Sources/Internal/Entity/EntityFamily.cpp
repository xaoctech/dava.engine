#include "EntityFamily.h"

namespace DAVA
{

EntityFamily::EntityFamily(uint32 family)
{
	currentSize = 0;
	maxSize = 0;
	ReallocatePools(15);
}


void EntityFamily::Flush()
{
	// 

	for (each moving entity in entities)
	{
		MoveToFamily(newFamily)
	}

	for (each deleted entity in entities)
	{
		Delete(entity);
	}

	if (currentSize + addedEntitiesSize > maxSize)
		ReallocatePools(currentSize + addedEntitiesSize);

	for (each added entity)
	{
		Add(entity);
	}

	/*
	Set<String name>

	for each data in set
	{
		if 
		PoolSystem::Instance()->CreatePool(entities.size());
	}
	*/

}

};
