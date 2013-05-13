#include "Scene3D/Components/SoundComponent.h"
#include "Sound/SoundEvent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA 
{

SoundComponent::SoundComponent()
{
    soundEvent = 0;
}

SoundComponent::~SoundComponent()
{
    SafeRelease(soundEvent);
}
    
void SoundComponent::SetSoundEvent(SoundEvent * sEvent)
{
	SafeRelease(soundEvent);
    soundEvent = SafeRetain(sEvent);
}
    
SoundEvent * SoundComponent::GetSoundEvent()
{
    return soundEvent;
}

const String & SoundComponent::GetEventName()
{
	return eventName;
}

void SoundComponent::SetEventName(const String & _eventName)
{
	DVASSERT(_eventName != "");

	eventName = _eventName;
	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::SOUND_CHANGED);
}

Component * SoundComponent::Clone(Entity * toEntity)
{
    SoundComponent * component = new SoundComponent();
	component->SetEntity(toEntity);

    //TODO: Do not forget ot check what does it means.
    component->soundEvent->fmodEvent = soundEvent->fmodEvent;
	component->eventName = eventName;
    return component;
}

void SoundComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);

	if(archive != 0 && soundEvent != 0)
	{
		archive->SetString("sc.eventName", eventName);
	}
}

void SoundComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(archive)
	{
		SetEventName(archive->GetString("sc.eventName"));
	}

	Component::Deserialize(archive, sceneFile);
}

};
