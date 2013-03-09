#include "Scene3D/Systems/SwitchSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Debug/Stats.h"

namespace DAVA
{

SwitchSystem::SwitchSystem(Scene * scene)
:	SceneSystem(scene)
{
	scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
}

void SwitchSystem::Process()
{
    TIME_PROFILE("SwitchSystem::Process");
	Set<SceneNode*>::iterator it;
	Set<SceneNode*>::const_iterator itEnd = updatableEntities.end();
	for(it = updatableEntities.begin(); it != itEnd; ++it)
	{
		SceneNode * entity = *it;
		SwitchComponent * sw = (SwitchComponent*)entity->GetComponent(Component::SWITCH_COMPONENT);

		if(sw->oldSwitchIndex != sw->newSwitchIndex)
		{
			int32 childrenCount = entity->GetChildrenCount();

			sw->newSwitchIndex = Clamp(sw->newSwitchIndex, 0, (childrenCount - 1));//start counting from zero

			for(int32 i = 0; i < childrenCount; ++i)
			{
				SetVisibleHierarchy(entity->GetChild(i), (sw->newSwitchIndex == i));
			}
			sw->oldSwitchIndex = sw->newSwitchIndex;
		}
	}

	updatableEntities.clear();
}

void SwitchSystem::ImmediateEvent(SceneNode * entity, uint32 event)
{
	if(EventSystem::SWITCH_CHANGED == event)
	{
		updatableEntities.insert(entity);
	}
}

void SwitchSystem::SetVisibleHierarchy(SceneNode * entity, bool visible)
{
	entity->SetSwitchVisible(visible);
	uint32 size = entity->GetChildrenCount();
	for(uint32 i = 0; i < size; ++i)
	{
		SetVisibleHierarchy(entity->GetChild(i), visible);
	}
}

}
