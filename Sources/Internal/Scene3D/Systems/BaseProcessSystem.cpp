#include "Scene3D/Systems/BaseProcessSystem.h"
#include "Scene3D/SceneNode.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

BaseProcessSystem::BaseProcessSystem(uint32 componentId)
:	processingComponentId(componentId)
{

}

void BaseProcessSystem::AddEntity(SceneNode * entity)
{
	components.push_back(entity->components[processingComponentId]);
}

void BaseProcessSystem::RemoveEntity(SceneNode * entity)
{
	uint32 size = components.size();
	Component * deletingComponent = entity->components[processingComponentId];
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