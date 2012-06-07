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
	enum eChangeState
	{
		FAMILY_CHANGED = 1
	};

	void AddComponent(Component * component);//const char * componentName);
	void AddComponent(const char * component);

	void RemoveComponent(Component * component);

    template<class T>
    void SetData(const char * dataName, const T & value);
    

    void SetFamily(EntityFamilyType newFamily);
	const EntityFamilyType & GetFamily();

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
	if(changeState & FAMILY_CHANGED)
	{
		DVASSERT(0 && "Entity::SetData called before manager->Flush()");
		Logger::Error("Entity::SetData called before manager->Flush()");
	}

    EntityFamily * enFamily = manager->GetFamilyByType(family);
	if(0 == enFamily)
	{
		Logger::Error("Entity::SetData enFamily==0");
		return;
	}

    T * t = enFamily->GetPtr<T>(dataName);
    t[indexInFamily] = value;
}
};

#endif // __DAVAENGINE_ENTITY_H__
