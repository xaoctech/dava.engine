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

#include "Sound/FMODSound.h"
#include "Sound/FMODSoundSystem.h"
#include "Sound/FMODUtils.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
Map<String, FMOD::Sound*> soundMap;
Map<FMOD::Sound*, int32> soundRefsMap;

FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

FMODSound * FMODSound::CreateWithFlags(const FilePath & fileName, uint32 flags, int32 priority /* = 128 */)
{
    FMODSoundSystem * soundSystem = FMODSoundSystem::GetFMODSoundSystem();
    
	FMODSound * sound = new FMODSound(fileName, priority);

    FMOD_MODE fmodMode = FMOD_DEFAULT;
    
	if(flags & SOUND_EVENT_CREATE_3D)
    {
		sound->is3D = true;
        fmodMode |= FMOD_3D;
    }
    
    Map<String, FMOD::Sound*>::iterator it;
    it = soundMap.find(fileName.GetAbsolutePathname());
    if (it != soundMap.end())
    {
        sound->fmodSound = it->second;
        soundRefsMap[sound->fmodSound]++;
    }

    if(!sound->fmodSound)
    {
        File * file = File::Create(fileName, File::OPEN | File::READ);
        if(!file)
        {
            SafeRelease(sound);
            return 0;
        }

        int32 fileSize = file->GetSize();
        if(!fileSize)
        {
            SafeRelease(sound);
            return 0;
        }

        sound->soundData = new uint8[fileSize];
        file->Read(sound->soundData, fileSize);
        SafeRelease(file);

        FMOD_CREATESOUNDEXINFO exInfo;
        memset(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exInfo.length = fileSize;

        if(flags & SOUND_EVENT_CREATE_STREAM)
        {
		    FMOD_VERIFY(soundSystem->fmodSystem->createStream((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
        }
        else
        {
            FMOD_VERIFY(soundSystem->fmodSystem->createSound((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | fmodMode, &exInfo, &sound->fmodSound));
            SafeDelete(sound->soundData);
        }

        if(!sound->fmodSound)
        {
            SafeRelease(sound);
            return 0;
        }

#if !defined DONT_USE_DEFAULT_3D_SOUND_SETTINGS
        if(sound->is3D && sound->fmodSound)
            FMOD_VERIFY(sound->fmodSound->set3DMinMaxDistance(5.0f, 100.0f));
#endif

        if(flags & SOUND_EVENT_CREATE_LOOP)
            sound->SetLoopCount(-1);
        else
            sound->SetLoopCount(0);

        soundMap[sound->fileName.GetAbsolutePathname()] = sound->fmodSound;
        soundRefsMap[sound->fmodSound] = 1;
    }
	FMOD_VERIFY(soundSystem->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));

	return sound;
}
    
FMODSound::FMODSound(const FilePath & _fileName, int32 _priority) :
    fileName(_fileName),
	priority(_priority),
	is3D(false),
    soundData(0),
    fmodSound(0),
    fmodInstanceGroup(0)
{
}

FMODSound::~FMODSound()
{
    SafeDeleteArray(soundData);

    FMOD_VERIFY(fmodInstanceGroup->release());

    FMODSoundSystem::GetFMODSoundSystem()->RemoveSoundEventFromGroups(this);
}

int32 FMODSound::Release()
{
    if(GetRetainCount() == 1)
    {
        soundRefsMap[fmodSound]--;
        if(soundRefsMap[fmodSound] == 0)
        {
            soundMap.erase(fileName.GetAbsolutePathname());
            soundRefsMap.erase(fmodSound);
            FMOD_VERIFY(fmodSound->release());
        }
    }

    return BaseObject::Release();
}

bool FMODSound::Trigger()
{
    FMOD::Channel * fmodInstance = 0;
    FMOD_VERIFY(FMODSoundSystem::GetFMODSoundSystem()->fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmodInstance)); //start sound paused
    FMOD_VECTOR pos = {position.x, position.y, position.z};
    FMOD_VERIFY(fmodInstance->setPriority(priority));
    FMOD_VERIFY(fmodInstance->setCallback(SoundInstanceEndPlaying));
    FMOD_VERIFY(fmodInstance->setUserData(this));
    FMOD_VERIFY(fmodInstance->setChannelGroup(fmodInstanceGroup));

    if(is3D)
        FMOD_VERIFY(fmodInstance->set3DAttributes(&pos, 0));
    
    FMOD_VERIFY(fmodInstanceGroup->setPaused(false));
    FMOD_VERIFY(fmodInstance->setPaused(false));

    Retain();

    return true;
}

void FMODSound::SetPosition(const Vector3 & _position)
{
	position = _position;
}

void FMODSound::UpdateInstancesPosition()
{
	if(is3D)
	{
		FMOD_VECTOR pos = {position.x, position.y, position.z};
		int32 instancesCount = 0;
		FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&instancesCount));
		for(int32 i = 0; i < instancesCount; i++)
		{
			FMOD::Channel * inst = 0;
			FMOD_VERIFY(fmodInstanceGroup->getChannel(i, &inst));
			FMOD_VERIFY(inst->set3DAttributes(&pos, 0));
		}
	}
}

void FMODSound::SetVolume(float32 volume)
{
	FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
}

float32	FMODSound::GetVolume()
{
	float32 volume = 0;
	FMOD_VERIFY(fmodInstanceGroup->getVolume(&volume));
	return volume;
}

bool FMODSound::IsActive()
{
    int32 numChanels = 0;
    FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&numChanels));
    
    bool isPaused = false;
    FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));
    
	return numChanels != 0 && !isPaused;
}

void FMODSound::Stop()
{
    FMOD_VERIFY(fmodInstanceGroup->stop());
}

int32 FMODSound::GetLoopCount()
{
	int32 loopCount;
	FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
	return loopCount;
}

void FMODSound::SetLoopCount(int32 loopCount)
{
	FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
}

void FMODSound::Pause()
{
    FMOD_VERIFY(fmodInstanceGroup->setPaused(true));
}

void FMODSound::PerformCallback(FMOD::Channel * instance)
{
    PerformEvent(EVENT_END);

    FMODSoundSystem::GetFMODSoundSystem()->ReleaseOnUpdate(this);
}

FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	if(type == FMOD_CHANNEL_CALLBACKTYPE_END)
	{
		FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
        if(cppchannel)
        {
            FMODSound * sound = 0;
            FMOD_VERIFY(cppchannel->getUserData((void**)&sound));
            if(sound)
                sound->PerformCallback(cppchannel);
        }
	}

	return FMOD_OK;
}

};
