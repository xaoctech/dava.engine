#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"
#include "Entity/ComponentTypes.h"

namespace DAVA 
{
    

class EntityManager;


class Component
{
public:
	static void RegisterComponent(const char * componentName, Component * component); //create or get from cache
	static Component * GetComponent(const char * componentName);
    static Component * Get();
    static Component * instance;
    
    Component()
    {
        type = ComponentType();
        componentsByIndex[type.GetIndex()] = this;
    };
    
    template <class T>
    void RegisterData(const char * name)
    {
        Pool * pool = new TemplatePool<T>(1);
        pools[name] = pool;
    }
    
//	template<class T>
//	TemplatePool<T>* CreatePool(T a, const char * name)
//	{
//		TemplatePool<T> * pool = new TemplatePool<T>(100);
//		pools.push_back(pool);
//        return pool;
//	}
//    
    template<class T>
	Vector<TemplatePool<T>*>* LinkToAllPools(T a, const char * name)
	{
        Vector<TemplatePool<T>*>* allPools;
        return allPools;
	}

    const ComponentType & GetType() { return type; };
    
    static Component * GetComponentByIndex(uint64 index);
    
    Map<const char *, Pool *> & GetNamedPools() { return pools; };
    uint32 GetPoolCount();
    
private:
	ComponentType type;
    Map<const char *, Pool *> pools;
    static Map<uint64, Component*>  componentsByIndex;
    static Map<const char *, Component * > cache;//<name, component>
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
