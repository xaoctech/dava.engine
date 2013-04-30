#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{

REGISTER_CLASS(SwitchComponent)


SwitchComponent::SwitchComponent()
:	oldSwitchIndex(-1),
	newSwitchIndex(0)
{

}

Component * SwitchComponent::Clone(Entity * toEntity)
{
	SwitchComponent * newComponent = new SwitchComponent();
	newComponent->SetEntity(toEntity);
	GlobalEventSystem::Instance()->Event(toEntity, this, EventSystem::SWITCH_CHANGED);
	return newComponent;
}

void SwitchComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);

	if(NULL != archive)
	{
		archive->SetInt32("sc.switchindex", newSwitchIndex);
	}
}

void SwitchComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		if(archive->IsKeyExists("sc.switchindex")) SetSwitchIndex(archive->GetInt32("sc.switchindex"));
	}

	Component::Deserialize(archive, sceneFile);
}

void SwitchComponent::SetSwitchIndex(const int32 & _switchIndex)
{
	newSwitchIndex = _switchIndex;

	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::SWITCH_CHANGED);
}

int32 SwitchComponent::GetSwitchIndex() const
{
	return newSwitchIndex;
}

}