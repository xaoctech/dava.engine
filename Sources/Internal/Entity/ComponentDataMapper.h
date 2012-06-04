#ifndef __DAVAENGINE_ENTITY_COMPONENT_DATA_MAPPER_H__
#define __DAVAENGINE_ENTITY_COMPONENT_DATA_MAPPER_H__

#include "Entity/Pool.h"
#include "Entity/EntityManager.h"

namespace DAVA 
{
        
template<class T>
class ComponentDataMapper
{
public:
    List<TemplatePool<T> *> poolList;
    
    ComponentDataMapper(EntityManager * manager, Component * component, const char * dataName)
    {
        manager->GetLinkedTemplatePoolsForComponent(component, dataName, poolList);
    }
    
    
    ComponentDataMapper(EntityManager * manager, EntityFamily * family)
    {
        
    }
    
};
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_DATA_MAPPER_H__
