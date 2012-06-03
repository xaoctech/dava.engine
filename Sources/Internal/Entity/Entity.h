#ifndef __DAVAENGINE_ENTITY_H__
#define __DAVAENGINE_ENTITY_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

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

	Vector<Component*> components;

	uint32 CalculateFamily();
	uint32 GetFamily();

	int32 GetChangeState();

	void SetIndex(int32 index);
	int32 GetIndex();
private:
	Entity(EntityManager * manager);
	int32 changeState;
	uint32 family;
	int32 indexInFamily;

	EntityManager * manager;

	friend class EntityManager;
};
};

#endif // __DAVAENGINE_ENTITY_H__
