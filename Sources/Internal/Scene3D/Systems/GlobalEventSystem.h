#ifndef __DAVAENGINE_SCENE3D_GLOBALEVENTSYSTEM_H__
#define __DAVAENGINE_SCENE3D_GLOBALEVENTSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/HashMap.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{
class Component;
class SceneNode;
class GlobalEventSystem : public StaticSingleton<GlobalEventSystem>
{
public:
    void Event(SceneNode * entity, Component * component, uint32 event);
    
    void PerformAllEventsFromCache(SceneNode * entity);
    void PerformAllEventsFromCache(Component * entity);
    
    void RemoveAllEvents(Component * component);
    
private:
    Map<Component*, List<uint32> > eventsCache;

};

}

#endif //__DAVAENGINE_SCENE3D_EVENTSYSTEM_H__