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
    
FMODSoundEvent::FMODSoundEvent(const String & eventName)
{
    FMOD_VERIFY(FMODSoundSystem::GetFMODSoundSystem()->fmodEventSystem->getEvent(eventName.c_str(), FMOD_EVENT_DEFAULT, &fmodEvent));
    if(fmodEvent)
    {
        FMOD_VERIFY(fmodEvent->setCallback(FMODEventCallback, this));
        FMODSoundSystem::GetFMODSoundSystem()->AddActiveFMODEvent(fmodEvent);
    }
}

FMODSoundEvent::~FMODSoundEvent()
{
    FMOD_VERIFY(fmodEvent->setCallback(0, 0));
    FMOD_VERIFY(fmodEvent->stop());
    FMODSoundSystem::GetFMODSoundSystem()->RemoveActiveFMODEvent(fmodEvent);
}

void FMODSoundEvent::Trigger()
{
	FMOD_VERIFY(fmodEvent->start());
}

void FMODSoundEvent::Stop()
{
	FMOD_VERIFY(fmodEvent->stop());
}

void FMODSoundEvent::SetVolume(float32 volume)
{
	FMOD_VERIFY(fmodEvent->setVolume(volume));
}

float32	FMODSoundEvent::GetVolume()
{
	float32 volume = 0;
	FMOD_VERIFY(fmodEvent->getVolume(&volume));
	return volume;
}

void FMODSoundEvent::Pause(bool isPaused)
{
	FMOD_VERIFY(fmodEvent->setPaused(isPaused));
}

bool FMODSoundEvent::IsPaused()
{
	bool isPaused = false;
	FMOD_VERIFY(fmodEvent->getPaused(&isPaused));
	return isPaused;
}

bool FMODSoundEvent::IsActive()
{
    if(fmodEvent)
    {
        FMOD_EVENT_STATE state;
        FMOD_VERIFY(fmodEvent->getState(&state));
        return (state & FMOD_EVENT_STATE_CHANNELSACTIVE) > 0;
    }
    return false;
}

void FMODSoundEvent::KeyOffParameter(const String & paramName)
{
    FMOD::EventParameter * param = 0;
    FMOD_VERIFY(fmodEvent->getParameter(paramName.c_str(), &param));
    if(param)
        FMOD_VERIFY(param->keyOff());
}

void FMODSoundEvent::PerformCallback(CallbackType callbackType)
{
    FMODSoundSystem::GetFMODSoundSystem()->PerformCallbackOnUpdate(this, callbackType);
}

FMOD_RESULT F_CALLBACK FMODEventCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
    if(type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED)
    {
        FMODSoundEvent * fevent = (FMODSoundEvent *)userdata;
        if(fevent)
            fevent->PerformCallback(FMODSoundEvent::EVENT_END);
    }

    return FMOD_OK;
}
    
};
