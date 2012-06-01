#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"

namespace DAVA 
{

class Component
{
public:
	static void RegisterComponent(const char * componentName, Component * component); //create or get from cache
	static Component * GetComponent(const char * componentName);
    static Component * Get();
    static Component * instance;
    
	template<class T>
	TemplatePool<T>* CreatePool(T a, const char * name)
	{
		TemplatePool<T> * pool = new TemplatePool<T>(100);
		pools.push_back(pool);
        return pool;
	}
    
    template<class T>
	Vector<TemplatePool<T>*>* LinkToAllPools(T a, const char * name)
	{
        Vector<TemplatePool<T>*>* allPools;
        return allPools;
	}
	
	Vector<Pool *> pools; // not dynamic can't be changed in runtime
private:
	static Map<const char *, Component * > cache;//<name, component>
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__