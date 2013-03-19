#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Scene.h"
#include "Entity/Component.h"

namespace DAVA
{
    

void GlobalEventSystem::Event(Entity * entity, Component * component, uint32 event)
{
    if (entity)
    {
        Scene * scene = entity->GetScene();
        if (scene)
        {
            scene->ImmediateEvent(entity, component->GetType(), event);
            return;
        }
    }
    
	List<uint32> & list = eventsCache[component];
	list.push_back(event);
}
    
void GlobalEventSystem::PerformAllEventsFromCache(Entity * entity)
{
    for (uint32 k = 0; k < Component::COMPONENT_COUNT; ++k)
    {
        Component * component = entity->GetComponent(k);
        if (component)
            PerformAllEventsFromCache(component);
    }
}
    
void GlobalEventSystem::PerformAllEventsFromCache(Component * component)
{
    Map<Component*, List<uint32> >::iterator it = eventsCache.find(component);
    if (it != eventsCache.end())
    {
        List<uint32> & list = it->second;
        
        for (List<uint32>::iterator listIt = list.begin(); listIt != list.end();  ++listIt)
        {
            component->GetEntity()->GetScene()->ImmediateEvent(component->GetEntity(), component->GetType(), *listIt);
        }
        
        eventsCache.erase(it);
    }
}


}