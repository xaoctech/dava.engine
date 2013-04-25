/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Igor Solovey
=====================================================================================*/

#include "Base/FastName.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundGroup.h"
#include "Sound/FMODUtils.h"

#ifdef __DAVAENGINE_IPHONE__
#include "fmodiphone.h"
#endif

namespace DAVA
{
SoundSystem::SoundSystem(int32 maxChannels)
{
	FMOD_VERIFY(FMOD::System_Create(&fmodSystem));
	FMOD_VERIFY(fmodSystem->init(maxChannels, FMOD_INIT_NORMAL, 0));
}

SoundSystem::~SoundSystem()
{
	for(Map<int, SoundGroup*>::iterator it = soundGroups.begin(); it != soundGroups.end(); it++)
	{
		SafeDelete(it->second);
	}

	FMOD_VERIFY(fmodSystem->release());
}

void SoundSystem::Update()
{
	for(Map<int, SoundGroup*>::iterator it = soundGroups.begin(); it != soundGroups.end(); it++)
	{
		it->second->Update();
	}

	fmodSystem->update();
}

void SoundSystem::Suspend()
{

}

void SoundSystem::Resume()
{
#ifdef __DAVAENGINE_IPHONE__
    FMOD_IPhone_RestoreAudioSession();
#endif
}

void SoundSystem::SetListenerPosition(const Vector3 & position)
{
	FMOD_VECTOR pos = {position.x, position.y, position.z};
	FMOD_VERIFY(fmodSystem->set3DListenerAttributes(0, &pos, 0, 0, 0));
}

void SoundSystem::SetListenerOrientation(const Vector3 & _at, const Vector3 & _up)
{
	FMOD_VECTOR at = {_at.x, _at.y, _at.z};
	FMOD_VECTOR up = {_up.x, _up.y, _up.z};
	FMOD_VERIFY(fmodSystem->set3DListenerAttributes(0, 0, 0, &at, &up));
}

SoundGroup * SoundSystem::GetSoundGroup(const FastName & groupName)
{
	if(soundGroups.find(groupName.Index()) == soundGroups.end())
		return 0;
	else
		return soundGroups[groupName.Index()];
}

SoundGroup * SoundSystem::CreateSoundGroup(const FastName & groupName)
{
	SoundGroup * group = 0;
	if(soundGroups.find(groupName.Index()) == soundGroups.end())
	{
		group = new SoundGroup();
		soundGroups[groupName.Index()] = group;
	}
	else
	{
		group = soundGroups[groupName.Index()];
	}

	return group;
}

};
