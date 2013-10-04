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
#include "Base/FastNameMap.h"
#include "Sound/Sound.h"
#include "Sound/SoundSystem.h"
#include "Sound/FMODSoundEvent.h"

namespace FMOD
{
class System;
class EventSystem;
class EventGroup;
};

namespace DAVA
{
class FMODSound;
class FMODSoundGroup;
class FMODSoundComponent;
class VolumeAnimatedObject;
class FMODSoundSystem : public SoundSystemInstance
{
public:
	FMODSoundSystem(int32 maxChannels = 64);
	virtual ~FMODSoundSystem();

    virtual Sound * CreateSound(const FilePath & fileName, Sound::eType type, const FastName & groupName, int32 priority = 128);
    virtual Sound * CreateSound3D(const FilePath & fileName, Sound::eType type, const FastName & groupName, int32 priority = 128);
    virtual Component * CreateSoundComponent();

	virtual void Update();
	virtual void Suspend();
	virtual void Resume();

	virtual void SetListenerPosition(const Vector3 & position);
	virtual void SetListenerOrientation(const Vector3 & at, const Vector3 & left);

    virtual void SetGroupVolume(const FastName & groupName, float32 volume);
    virtual float32 GetGroupVolume(const FastName & groupName);
    virtual void StopGroup(const FastName & groupName);

    void LoadAllFEVsRecursive(const DAVA::FilePath & dirPath);

	void LoadFEV(const FilePath & filePath);
    void UnloadProjects();

    void PreloadEventGroupData(const String & groupName);
    void ReleaseEventGroupData(const String & groupName);
    
    void GetAllEventsNames(Vector<String> & names);

protected:
    FMODSoundGroup * CreateSoundGroup(const FastName & groupName);
    FMODSoundGroup * GetSoundGroup(const FastName & groupName);

    void ReleaseOnUpdate(FMODSound * sound);
    void GetGroupEventsNamesRecursive(FMOD::EventGroup * group, String & currNamePath, Vector<String> & names);

    void PerformCallbackOnUpdate(FMODSoundEvent * event, FMODSoundEvent::CallbackType type);

	FMOD::System * fmodSystem;
	FMOD::EventSystem * fmodEventSystem;

    Vector<FMODSound *> soundsToReleaseOnUpdate;
    FastNameMap<FMODSoundGroup *> soundGroups;
    Map<FMODSoundEvent *, FMODSoundEvent::CallbackType> callbackOnUpdate;

friend class FMODSoundGroup;
friend class FMODSound;
friend class FMODSoundEvent;
friend class FMODSoundComponent;
friend class VolumeAnimatedObject;
};



};

#endif //__DAVAENGINE_SOUND_SYSTEM_H__
