#include "Scene3D/Systems/BaseProcessSystem.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

BaseProcessSystem::BaseProcessSystem(uint32 componentId, Scene * scene)
:	SceneSystem(scene),
	processingComponentId(componentId)
{

}

void BaseProcessSystem::AddEntity(Entity * entity)
{
	components.push_back(entity->GetComponent(processingComponentId));
}

void BaseProcessSystem::RemoveEntity(Entity * entity)
{
	uint32 size = components.size();
	Component * deletingComponent = entity->GetComponent(processingComponentId);
	for(uint32 i = 0; i < size; ++i)
	{
		if(components[i] == deletingComponent)
		{
			components[i] = components[size-1];
			components.pop_back();
			return;
		}
	}

	DVASSERT(0);
}

}