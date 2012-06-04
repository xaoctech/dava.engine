#ifndef __DAVAENGINE_ENTITY_H__
#define __DAVAENGINE_ENTITY_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Entity/ComponentType.h"

namespace DAVA 
{
class EntityManager;
class Entity
{
public:
	enum eChangeParameter
	{
		CREATED			= 1 << 0,
		COMPONENT_ADDED = 1 << 1
	};

	void AddComponent(Component * component);//const char * componentName);

	//Vector<Component*> components;

    void SetFamily(EntityFamilyType newFamily);
	const EntityFamilyType & GetFamily();

	int32 GetChangeState();

	void SetIndexInFamily(int32 index);
	int32 GetIndexInFamily();
private:
	Entity(EntityManager * manager);
	EntityFamilyType  family;
	int32   indexInFamily;

	int32 changeState;

	EntityManager * manager;

	friend class EntityManager;
};
};

#endif // __DAVAENGINE_ENTITY_H__
