#include "Scene3D/Systems/EventSystem.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
    
void EventSystem::RegisterSystemForEvent(SceneSystem * system, uint32 event)
{
	registeredSystems[event].push_back(system);
}

void EventSystem::UnregisterSystemForEvent(SceneSystem * system, uint32 event)
{
	Vector<SceneSystem*> & container = registeredSystems[event];
	uint32 size = container.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(container[i] == system)
		{
			container[i] = container[size-1];
			container.pop_back();
			return;
		}
	}
}

void EventSystem::NotifySystem(SceneSystem * system, Entity * entity, uint32 event)
{
	Vector<SceneSystem*> & container = registeredSystems[event];
	uint32 size = container.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(system == container[i])
		{
			system->ImmediateEvent(entity, event);
			return;
		}
	}
}

}