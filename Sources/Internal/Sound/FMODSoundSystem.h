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

#ifndef __DAVAENGINE_FMOD_SOUND_SYSTEM_H__
#define __DAVAENGINE_FMOD_SOUND_SYSTEM_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Sound/FMODSoundEvent.h"

namespace FMOD
{
class System;
class EventSystem;
class EventGroup;
};

namespace DAVA
{
   
class FMODSoundSystem : public SoundSystemInstance
{
    struct SoundGroup
    {
        FastName name;
        float32 volume;
        Vector<SoundEvent *> events;
    };

public:
	FMODSoundSystem(int32 maxChannels = 64);
	virtual ~FMODSoundSystem();

    virtual SoundEvent * CreateSoundEventByID(const String & eventName, const FastName & groupName);
    virtual SoundEvent * CreateSoundEventFromFile(const FilePath & fileName, const FastName & groupName, uint32 createFlags = SoundEvent::SOUND_EVENT_CREATE_DEFAULT, int32 priority = 128);

	virtual void Update(float32 timeElapsed);
	virtual void Suspend();
	virtual void Resume();

    virtual void SetCurrentLocale(const String & langID);

	virtual void SetListenerPosition(const Vector3 & position);
	virtual void SetListenerOrientation(const Vector3 & forward, const Vector3 & left);

    virtual void SetGroupVolume(const FastName & groupName, float32 volume);
    virtual float32 GetGroupVolume(const FastName & groupName);

    //FMOD Only
    static FMODSoundSystem * GetFMODSoundSystem();

    void LoadAllFEVsRecursive(const DAVA::FilePath & dirPath);

	void LoadFEV(const FilePath & filePath);
    void UnloadFMODProjects();

    void PreloadFMODEventGroupData(const String & groupName);
    void ReleaseFMODEventGroupData(const String & groupName);
    
    void GetAllEventsNames(Vector<String> & names);

    void SetMaxDistance(float32 distance);
    float32 GetMaxDistanceSquare();

    uint32 GetMemoryUsageBytes();
    
protected:
    void GetGroupEventsNamesRecursive(FMOD::EventGroup * group, String & currNamePath, Vector<String> & names);
    
    void ReleaseOnUpdate(SoundEvent * sound);
    
    void AddSoundEventToGroup(const FastName & groupName, SoundEvent * event);
    void RemoveSoundEventFromGroups(SoundEvent * event);
    
	FMOD::System * fmodSystem;
	FMOD::EventSystem * fmodEventSystem;

    Vector<SoundEvent *> soundsToReleaseOnUpdate;

    Vector<SoundGroup> soundGroups;

    float32 maxDistanceSq;
    Vector3 listenerPosition;

friend class FMODSound;
friend class FMODSoundEvent;
friend class FMODSoundComponent;
};

};

#endif //__DAVAENGINE_SOUND_SYSTEM_H__
