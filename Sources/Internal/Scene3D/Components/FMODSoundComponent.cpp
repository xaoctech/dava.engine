/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/FMODSoundComponent.h"
#include "Sound/SoundSystem.h"
#include "Sound/FMODSoundSystem.h"
#include "Sound/FMODSoundEvent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Entity.h"

namespace DAVA 
{

FMODSoundComponent::FMODSoundComponent()
{
    soundEvent = 0;
}

FMODSoundComponent::~FMODSoundComponent()
{
    SafeRelease(soundEvent);
}
    
void FMODSoundComponent::SetSoundEvent(FMODSoundEvent * sEvent)
{
	SafeRelease(soundEvent);
    soundEvent = SafeRetain(sEvent);
}
    
FMODSoundEvent * FMODSoundComponent::GetSoundEvent()
{
    return soundEvent;
}

const String & FMODSoundComponent::GetEventName()
{
	return eventName;
}

void FMODSoundComponent::SetEventName(const String & _eventName)
{
	DVASSERT(_eventName != "");
    if(eventName == _eventName && soundEvent)
        return;

	eventName = _eventName;
	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::SOUND_CHANGED);
}

Component * FMODSoundComponent::Clone(Entity * toEntity)
{
    FMODSoundComponent * component = new FMODSoundComponent();
	component->SetEntity(toEntity);

    //TODO: Do not forget ot check what does it means.
    if(soundEvent)
    {
        FMODSoundSystem * soundSystem = (FMODSoundSystem *)SoundSystem::Instance();
        DVASSERT(soundSystem);
        component->soundEvent = soundSystem->CreateSoundEvent(eventName);
    }
	component->eventName = eventName;
    return component;
}

void FMODSoundComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	SoundComponent::Serialize(archive, sceneFile);

	if(archive != 0)
	{
		archive->SetString("sc.eventName", eventName);
	}
}

void FMODSoundComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(archive)
	{
		SetEventName(archive->GetString("sc.eventName"));
	}

	SoundComponent::Deserialize(archive, sceneFile);
}

void FMODSoundComponent::Play()
{
    if(soundEvent)
        soundEvent->Play();
}

void FMODSoundComponent::Stop()
{
    if(soundEvent)
        soundEvent->Stop();
}

void FMODSoundComponent::SetParameter(const String & paramName, float32 value)
{
    if(soundEvent)
        soundEvent->SetParameterValue(paramName, value);
}
float32 FMODSoundComponent::GetParameter(const String & paramName)
{
    if(soundEvent)
        return soundEvent->GetParameterValue(paramName);
    return 0.f;
}
void FMODSoundComponent::SetPosition(const Vector3 & position)
{
    if(soundEvent)
        soundEvent->SetPosition(position);
}

};
