#ifndef __DAVAENGINE_SCENE3D_EVENTSYSTEM_H__
#define __DAVAENGINE_SCENE3D_EVENTSYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class SceneSystem;
class SceneNode;
class EventSystem
{
public:
	enum eEventType
	{
		LOCAL_TRANSFORM_CHANGED = 0,
		WORLD_TRANSFORM_CHANGED,

		EVENTS_COUNT
	};

	void RegisterSystemForEvent(SceneSystem * system, uint32 event);
	void UnregisterSystemForEvent(SceneSystem * system, uint32 event);
	void NotifySystem(SceneSystem * system, SceneNode * entity, uint32 event);

private:
	Vector<SceneSystem*> registeredSystems[EVENTS_COUNT];
};

}

#endif //__DAVAENGINE_SCENE3D_EVENTSYSTEM_H__