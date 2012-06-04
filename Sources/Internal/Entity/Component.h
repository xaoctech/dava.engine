#ifndef __DAVAENGINE_ENTITY_COMPONENT_H__
#define __DAVAENGINE_ENTITY_COMPONENT_H__

#include "Entity/Pool.h"
#include "Entity/ComponentTypes.h"
#include "Entity/EntityManager.h"
#include <typeinfo>


namespace DAVA 
{

class Component
{
public:
	static void RegisterComponent(const char * componentName, Component * component); //create or get from cache
	static Component * GetComponent(const char * componentName);
    static Component * Get();
    static Component * instance;
    
    Component()
    {
        //type = ComponentType(); //this duplicates field declaration
        componentsByIndex[type.GetIndex()] = this;
    };
    
    
    template <class T>
    void RegisterData(const char * name)
    {
        EntityManager::CreatePoolAllocator<T>(name);
        dataNames.insert(name);
    }

//	template<class T>
//	TemplatePool<T>* CreatePool(T a, const char * name)
//	{
//		TemplatePool<T> * pool = new TemplatePool<T>(100);
//		pools.push_back(pool);
//        return pool;
//	}
//    

    const ComponentType & GetType() { return type; };
    
    static Component * GetComponentByIndex(uint64 index);
    
    Set<const char*> & GetDataNames() {return dataNames; };
    
private:
	ComponentType type;
    Set<const char*> dataNames;
    static Map<uint64, Component*>  componentsByIndex;
    static Map<const char *, Component * > cache;//<name, component>
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
