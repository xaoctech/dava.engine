#include "Entity/Component.h"

namespace DAVA 
{

Map<const char *, Component * > Component::cache;
Component * Component::instance = 0;

Component * Component::Get()
{
    return instance;
}
void Component::RegisterComponent(const char * componentName, Component * component)
{
    cache[componentName] = component;
}
    
Component * Component::GetComponent(const char * componentName)
{
    Map<const char *, Component * >::iterator res = cache.find(componentName);
    if (res != cache.end())
    {
        return res->second;
    }
    return 0;
}
    
};

