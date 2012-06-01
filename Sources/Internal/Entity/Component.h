#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"

namespace DAVA 
{

class Component
{
public:
	static Component * CreateComponent(const char * componentName); //create or get from cache
	
	template<class T>
	void CreatePool(const char * type, const char * name)
	{
		TemplatePool<T> * pool = new TemplatePool<T>(100);
		pools.push_back(pool);
	}
	
	Vector<Pool *> pools; // not dynamic can't be changed in runtime
private:
	static Map<const char *, Component * > cache;//<name, component>
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__