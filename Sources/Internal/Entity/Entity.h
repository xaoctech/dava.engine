#ifndef __DAVAENGINE_ENTITY_H__
#define __DAVAENGINE_ENTITY_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

namespace DAVA 
{

class Entity
{
public:
	void AddComponent(Component * component);//const char * componentName);

	int32 family;
private:
	Vector<Component*> components;
};
};

#endif // __DAVAENGINE_ENTITY_H__