#include "Scene3D/Systems/UpdatableSystem.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/SceneNode.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

UpdatableSystem::UpdatableSystem()
{

}

void UpdatableSystem::AddEntity(SceneNode * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();
	DVASSERT(object);

	IUpdatablePreTransform * updatePreTransform = dynamic_cast<IUpdatablePreTransform*>(object);
	if(updatePreTransform)
	{
		updatesPreTransform.push_back(updatePreTransform);
	}

	IUpdatablePostTransform * updatePostTransform = dynamic_cast<IUpdatablePostTransform*>(object);
	if(updatePostTransform)
	{
		updatesPostTransform.push_back(updatePostTransform);
	}
}

void UpdatableSystem::RemoveEntity(SceneNode * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();

	if(object)
	{
		IUpdatablePreTransform * updatePreTransform = dynamic_cast<IUpdatablePreTransform*>(object);
		if(updatePreTransform)
		{
			uint32 size = updatesPreTransform.size();
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesPreTransform[i] == updatePreTransform)
				{
					updatesPreTransform[i] = updatesPreTransform[size-1];
					updatesPreTransform.pop_back();
					return;
				}
			}
		}

		IUpdatablePostTransform * updatePostTransform = dynamic_cast<IUpdatablePostTransform*>(object);
		if(updatePostTransform)
		{
			uint32 size = updatesPostTransform.size();
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesPostTransform[i] == updatePostTransform)
				{
					updatesPostTransform[i] = updatesPostTransform[size-1];
					updatesPostTransform.pop_back();
					return;
				}
			}
		}
	}
}

void UpdatableSystem::Process()
{

}

void UpdatableSystem::UpdatePreTransform()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	uint32 size = updatesPreTransform.size();
	for(uint32 i = 0; i < size; ++i)
	{
		updatesPreTransform[i]->UpdatePreTransform(timeElapsed);
	}
}

void UpdatableSystem::UpdatePostTransform()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	uint32 size = updatesPostTransform.size();
	for(uint32 i = 0; i < size; ++i)
	{
		updatesPostTransform[i]->UpdatePostTransform(timeElapsed);
	}
}

}