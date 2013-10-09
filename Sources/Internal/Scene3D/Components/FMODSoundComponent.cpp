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
#include "Sound/FMODUtils.h"

namespace DAVA 
{

FMOD_RESULT F_CALLBACK FMODComponentEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata);
    
FMODSoundComponent::FMODSoundComponent() : fmodEvent(0)
{
    
}

FMODSoundComponent::~FMODSoundComponent()
{
    if(fmodEvent)
    {
        FMOD_VERIFY(fmodEvent->setCallback(0, 0));
        FMOD_VERIFY(fmodEvent->stop());
        FMODSoundSystem::GetFMODSoundSystem()->RemoveActiveFMODEvent(fmodEvent);
    }
}

const String & FMODSoundComponent::GetEventName()
{
	return eventName;
}

void FMODSoundComponent::SetEventName(const String & _eventName)
{
	DVASSERT(_eventName != "");
    if(eventName == _eventName)
        return;

    if(_eventName[0] == '/')
        eventName = _eventName.substr(1);
    else
	    eventName = _eventName;

	GlobalEventSystem::Instance()->Event(entity, this, EventSystem::SOUND_CHANGED);
}

Component * FMODSoundComponent::Clone(Entity * toEntity)
{
    FMODSoundComponent * component = new FMODSoundComponent();
	component->SetEntity(toEntity);

    //TODO: Do not forget ot check what does it means.
    if(fmodEvent)
    {
        FMOD_VERIFY(FMODSoundSystem::GetFMODSoundSystem()->fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_DEFAULT, &component->fmodEvent));
        if(component->fmodEvent)
            FMODSoundSystem::GetFMODSoundSystem()->AddActiveFMODEvent(component->fmodEvent);
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

bool FMODSoundComponent::Trigger()
{
    FMOD::EventSystem * fmodEventSystem = FMODSoundSystem::GetFMODSoundSystem()->fmodEventSystem;
    
    if((position - FMODSoundSystem::GetFMODSoundSystem()->listenerPosition).SquareLength() > FMODSoundSystem::GetFMODSoundSystem()->maxDistanceSq)
        return false;

    if(!fmodEvent)
    {
        FMOD::Event * fmodEventInfo = 0;
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
        FMOD_VECTOR pos = {position.x, position.y, position.z};
        FMOD_VERIFY(fmodEventInfo->set3DAttributes(&pos, 0));
        
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_DEFAULT, &fmodEvent));
        if(fmodEvent)
        {
            FMOD_VERIFY(fmodEvent->setCallback(FMODComponentEventCallback, &fmodEvent));
            FMODSoundSystem::GetFMODSoundSystem()->AddActiveFMODEvent(fmodEvent);
        }
    }
    if(fmodEvent)
    {
        FMOD_VERIFY(fmodEvent->start());
        return true;
    }

    return false;
}

void FMODSoundComponent::Stop()
{
    if(fmodEvent)
    {
        FMOD_VERIFY(fmodEvent->stop());
    }
}

void FMODSoundComponent::SetParameter(const String & paramName, float32 value)
{
    if(fmodEvent)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(fmodEvent->getParameter(paramName.c_str(), &param));
        if(param)
            FMOD_VERIFY(param->setValue(value));
    }
}
    
float32 FMODSoundComponent::GetParameter(const String & paramName)
{
    float32 returnValue = 0.f;
    if(fmodEvent)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(fmodEvent->getParameter(paramName.c_str(), &param));
        if(param)
            FMOD_VERIFY(param->getValue(&returnValue));
    }
    return returnValue;
}
    
void FMODSoundComponent::SetPosition(const Vector3 & _position)
{
    position = _position;
    if(fmodEvent)
    {
        FMOD_VECTOR pos = {position.x, position.y, position.z};
        FMOD_VERIFY(fmodEvent->set3DAttributes(&pos, 0));
    }
}
    
void FMODSoundComponent::KeyOffParameter(const String & paramName)
{
    if(fmodEvent)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(fmodEvent->getParameter(paramName.c_str(), &param));
        if(param)
            FMOD_VERIFY(param->keyOff());
    }
}
    
void FMODSoundComponent::GetEventParametersInfo(Vector<SoundEventParameterInfo> & paramsInfo)
{
    paramsInfo.clear();
    
    bool isEventInfoOnly = false;

    FMOD::EventSystem * fmodEventSystem = FMODSoundSystem::GetFMODSoundSystem()->fmodEventSystem;
    FMOD::Event * event = fmodEvent;
    if(!event)
    {
        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &event));
        isEventInfoOnly = true;
    }
    if(!event)
    {
        return;
    }
    
    int32 paramsCount = 0;
    FMOD_VERIFY(event->getNumParameters(&paramsCount));
    for(int32 i = 0; i < paramsCount; i++)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(event->getParameterByIndex(i, &param));
        if(!param)
            continue;
        
        char * paramName = 0;
        FMOD_VERIFY(param->getInfo(0, &paramName));
        
        SoundEventParameterInfo pInfo;
        pInfo.name = String(paramName);
        FMOD_VERIFY(param->getRange(&pInfo.minValue, &pInfo.maxValue));

        if(!isEventInfoOnly)
            FMOD_VERIFY(param->getValue(&pInfo.currentValue));
            
        paramsInfo.push_back(pInfo);
    }
}

FMOD_RESULT F_CALLBACK FMODComponentEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
    if(type == FMOD_EVENT_CALLBACKTYPE_STOLEN || type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED)
    {
        FMOD::Event **event = (FMOD::Event **)userdata;
        FMODSoundSystem::GetFMODSoundSystem()->RemoveActiveFMODEvent((*event));
        (*event) = 0;
    }
    return FMOD_OK;
}
    
};
