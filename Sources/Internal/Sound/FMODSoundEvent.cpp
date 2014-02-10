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



#include "Sound/FMODSoundEvent.h"
#include "Sound/SoundSystem.h"
#include "Sound/FMODSoundSystem.h"
#include "Sound/FMODUtils.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
    
FMOD_RESULT F_CALLBACK FMODEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata);
    
FMODSoundEvent::FMODSoundEvent(const String & _eventName)
{
    if(_eventName.size())
    {
        if(_eventName[0] == '/')
            eventName = _eventName.substr(1);
        else
            eventName = _eventName;

        FMOD::Event * fmodEventInfo = 0;
        FMOD_VERIFY(FMODSoundSystem::GetFMODSoundSystem()->fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
        if(fmodEventInfo)
        {
            FMOD_MODE mode = 0;
            fmodEventInfo->getPropertyByIndex(FMOD_EVENTPROPERTY_MODE, &mode);
            is3D = (mode == FMOD_3D);
        }
        else
        {
            Logger::Debug(eventName.c_str());
        }
    }
}

FMODSoundEvent::~FMODSoundEvent()
{
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
    {
        FMOD::Event * fmodEvent = (*it);
        
        FMOD_VERIFY(fmodEvent->setCallback(0, 0));
        FMOD_VERIFY(fmodEvent->stop());

        FMODSoundSystem::GetFMODSoundSystem()->CancelCallbackOnUpdate(this, SoundEvent::EVENT_END);
    }

    fmodEventInstances.clear();

    FMODSoundSystem::GetFMODSoundSystem()->RemoveSoundEventFromGroups(this);
}

bool FMODSoundEvent::Trigger()
{
    FMODSoundSystem * fmodSoundSystem = FMODSoundSystem::GetFMODSoundSystem();
    FMOD::EventSystem * fmodEventSystem = fmodSoundSystem->fmodEventSystem;

    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
        (*it)->setPaused(false);
    
    if(is3D)
    {
        if((position - fmodSoundSystem->listenerPosition).SquareLength() > fmodSoundSystem->maxDistanceSq)
            return false;

        FMOD::Event * fmodEventInfo = 0;
        FMOD_VECTOR pos = {position.x, position.y, position.z};
        FMOD_VECTOR orient = {orientation.x, orientation.y, orientation.z};

        FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
        if(fmodEventInfo)
        {
            FMOD_VERIFY(fmodEventInfo->set3DAttributes(&pos, 0, &orient));
            FMOD_VERIFY(fmodEventInfo->setVolume(volume));
            ApplyParamsToEvent(fmodEventInfo);
        }
    }
    
    FMOD::Event * fmodEvent = 0;
    FMOD_RESULT result = fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_DEFAULT, &fmodEvent);
    if(fmodEvent)
    {
        ApplyParamsToEvent(fmodEvent);
        fmodEventInstances.push_back(fmodEvent);

        FMOD_VERIFY(fmodEvent->setVolume(volume));
        FMOD_VERIFY(fmodEvent->setCallback(FMODEventCallback, this));
        FMOD_VERIFY(fmodEvent->start());
        
        Retain();
    }
    else if(result != FMOD_ERR_EVENT_FAILED) //'just fail' max playbacks behavior
    {
        Logger::Debug("[FMODSoundEvent::Trigger()] Failed by %d on eventID: %s", result, eventName.c_str());
    }
    
    return fmodEvent != 0;
}
    
void FMODSoundEvent::SetPosition(const Vector3 & _position)
{
    position = _position;
}

void FMODSoundEvent::SetOrientation(const Vector3 & _orientation)
{
    orientation = _orientation;
    orientation.Normalize();
}

void FMODSoundEvent::SetVolume(float32 _volume)
{
    volume = _volume;
    
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
        FMOD_VERIFY((*it)->setVolume(volume));
}
    
float32	FMODSoundEvent::GetVolume()
{
    return volume;
}
    
void FMODSoundEvent::UpdateInstancesPosition()
{
    FMODSoundSystem * fmodSoundSystem = FMODSoundSystem::GetFMODSoundSystem();
    
    if(fmodEventInstances.size() && (fmodSoundSystem->listenerPosition - position).SquareLength() > fmodSoundSystem->maxDistanceSq)
        Stop();
    
    FMOD_VECTOR pos = {position.x, position.y, position.z};
    FMOD_VECTOR orient = {orientation.x, orientation.y, orientation.z};
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
        FMOD_VERIFY((*it)->set3DAttributes(&pos, 0, &orient));
}
    
void FMODSoundEvent::Stop()
{
    FMODSoundSystem * fmodSoundSystem = FMODSoundSystem::GetFMODSoundSystem();

    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
    {
        FMOD::Event * fEvent = (*it);
        FMOD_VERIFY(fEvent->setCallback(0, 0));
        FMOD_VERIFY(fEvent->stop());
        fmodSoundSystem->ReleaseOnUpdate(this);
        PerformEvent(SoundEvent::EVENT_END);
    }
    fmodEventInstances.clear();
}

bool FMODSoundEvent::IsActive()
{
    return fmodEventInstances.size() != 0;
}

void FMODSoundEvent::Pause()
{
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
        (*it)->setPaused(true);
}

void FMODSoundEvent::Serialize(KeyedArchive *archive)
{
    archive->SetString("fmSE.eventID", eventName);
}
    
void FMODSoundEvent::Deserialize(KeyedArchive *archive)
{
    eventName = archive->GetString("fmSE.eventID");
}
    
void FMODSoundEvent::SetParameterValue(const FastName & paramName, float32 value)
{
    paramsValues[paramName] = value;
    
    List<FMOD::Event *>::const_iterator itEnd = fmodEventInstances.end();
    for(List<FMOD::Event *>::const_iterator it = fmodEventInstances.begin(); it != itEnd; ++it)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY((*it)->getParameter(paramName.c_str(), &param));
        if(param)
        {
            FMOD_VERIFY(param->setValue(value));
        }
    }
}

float32 FMODSoundEvent::GetParameterValue(const FastName & paramName)
{
    return paramsValues[paramName];
}

bool FMODSoundEvent::IsParameterExists(const FastName & paramName)
{
    FMODSoundSystem * fmodSoundSystem = FMODSoundSystem::GetFMODSoundSystem();
    FMOD::EventSystem * fmodEventSystem = fmodSoundSystem->fmodEventSystem;

    FMOD::Event * fmodEventInfo = 0;
    FMOD_VERIFY(fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_INFOONLY, &fmodEventInfo));
    if(fmodEventInfo)
    {
        FMOD::EventParameter * param = 0;
        fmodEventInfo->getParameter(paramName.c_str(), &param);
        return param != NULL;
    }

    return false;
}

void FMODSoundEvent::ApplyParamsToEvent(FMOD::Event *event)
{
    FastNameMap<float32>::iterator it = paramsValues.begin();
    FastNameMap<float32>::iterator itEnd = paramsValues.end();
    while(it != itEnd)
    {
        FMOD::EventParameter * param = 0;
        FMOD_VERIFY(event->getParameter(it->first.c_str(), &param));
        if(param)
            FMOD_VERIFY(param->setValue(it->second));
        ++it;
    }
}
    
void FMODSoundEvent::PerformCallback(FMOD::Event * fmodEvent, SoundEventCallback callbackType)
{
    FMODSoundSystem * fmodSoundSystem = FMODSoundSystem::GetFMODSoundSystem();
    fmodSoundSystem->ReleaseOnUpdate(this);
    fmodSoundSystem->PerformCallbackOnUpdate(this, callbackType);
    
    List<FMOD::Event *>::iterator it = std::find(fmodEventInstances.begin(), fmodEventInstances.end(), fmodEvent);
    if(it != fmodEventInstances.end())
        fmodEventInstances.erase(it);
}

FMOD_RESULT F_CALLBACK FMODEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
    if(/*type == FMOD_EVENT_CALLBACKTYPE_STOLEN || */type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED)
    {
        FMOD::Event * fEvent = (FMOD::Event *)event;
        FMODSoundEvent * sEvent = (FMODSoundEvent *)userdata;
        if(sEvent && fEvent)
            sEvent->PerformCallback(fEvent, SoundEvent::EVENT_END);
    }
    return FMOD_OK;
}
    
};