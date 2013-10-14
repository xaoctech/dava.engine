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

#ifndef __DAVAENGINE_SOUND_SYSTEM_H__
#define __DAVAENGINE_SOUND_SYSTEM_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastNameMap.h"
#include "FileSystem/FilePath.h"
#include "Base/EventDispatcher.h"
#include "Sound/Sound.h"

#define DEFAULT_SOUNDS_DIRECTORY "~res:/Sfx/"

namespace DAVA
{

class Component;
class SoundSystemInstance
{
public:
    virtual ~SoundSystemInstance() {};
    
    virtual Sound * CreateSound(const FilePath & fileName, Sound::eType type, const FastName & groupName, bool is3D = false, int32 priority = 128) = 0;
    virtual Component * CreateSoundComponent() = 0;

    virtual void Update(float32 timeElapsed);
    virtual void Suspend() = 0;
    virtual void Resume() = 0;

    virtual void SetListenerPosition(const Vector3 & position) = 0;
    virtual void SetListenerOrientation(const Vector3 & forward, const Vector3 & left) = 0;

    virtual void StopGroup(const FastName & groupName) = 0;

    virtual void SetGroupVolume(const FastName & groupName, float32 volume) = 0;
    virtual float32 GetGroupVolume(const FastName & groupName) = 0;

    virtual void SetGlobalComponentsVolume(float32 volume) = 0;
    virtual float32 GetSoundComponentsVolume() = 0;

    virtual void SetMaxDistance(float32 distance) = 0;
    virtual float32 GetMaxDistance() = 0;

protected:
    void AddVolumeAnimatedObject(VolumeAnimatedObject * object);
    void RemoveVolumeAnimatedObject(VolumeAnimatedObject * object);

    Vector<VolumeAnimatedObject*> animatedObjects;

friend class VolumeAnimatedObject;
};

class SoundSystem
{
public:
    enum SoundSystemType
    {
        SOUNDSYSTEM_FMOD = 0,

        SOUNDSYSTEM_COUNT
    };

public:
    static void Init();
    static void Release();
    static SoundSystemInstance * Instance();
    static void SetSoundSystemType(SoundSystemType _type) {type = _type;};
    static const FilePath & GetSoundsDirectory();
    static void SetSoundsDirectory(const FilePath & soundsDir);

private:
    static SoundSystemType type;
    static SoundSystemInstance * instance;

    static FilePath soundsDir;
};

};

#endif //__DAVAENGINE_SOUND_SYSTEM_H__
