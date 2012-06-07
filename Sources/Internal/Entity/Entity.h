#ifndef __DAVAENGINE_ENTITY_H__
#define __DAVAENGINE_ENTITY_H__

#include "Base/BaseTypes.h"
#include "Entity/ComponentTypes.h"
#include "Entity/EntityFamily.h"
#include "Entity/EntityManager.h"

namespace DAVA 
{
class Component;
class Entity
{
public:
	enum eChangeParameter
	{
		CREATED			= 1 << 0,
		COMPONENT_ADDED = 1 << 1
	};

	void AddComponent(Component * component);//const char * componentName);
	void AddComponent(const char * component);

    template<class T>
    void SetData(const char * dataName, const T & value);
    

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
    
template<class T>
void Entity::SetData(const char * dataName, const T & value)
{
    EntityFamily * enFamily = manager->GetFamilyByType(family);
    T * t = enFamily->GetPtr<T>(dataName);
    t[indexInFamily] = value;
}
};

#endif // __DAVAENGINE_ENTITY_H__
