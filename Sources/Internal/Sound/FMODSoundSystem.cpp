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

#include "Base/FastName.h"
#include "Sound/FMODSoundSystem.h"
#include "Sound/FMODSoundGroup.h"
#include "Scene3D/Components/FMODSoundComponent.h"
#include "Sound/FMODUtils.h"
#include "Sound/FMODSound.h"
#include "FileSystem/FileList.h"
#include "Scene3D/Entity.h"

#ifdef __DAVAENGINE_IPHONE__
#include "fmodiphone.h"
#endif

namespace DAVA
{

FMODSoundSystem::FMODSoundSystem(int32 maxChannels /* = 64 */) : 
globalComponentsVolume(1.f),
maxDistanceSq(150.f * 150.f),
distanceUpdateTime(.0f)
{
	FMOD_VERIFY(FMOD::EventSystem_Create(&fmodEventSystem));
	FMOD_VERIFY(fmodEventSystem->getSystemObject(&fmodSystem));
#ifdef DAVA_FMOD_PROFILE
    FMOD_VERIFY(fmodEventSystem->init(maxChannels, FMOD_INIT_NORMAL | FMOD_INIT_ENABLE_PROFILE, 0));
#else
    FMOD_VERIFY(fmodEventSystem->init(maxChannels, FMOD_INIT_NORMAL, 0));
#endif
    FMOD_VERIFY(fmodSystem->set3DSettings(1.f, 1.f, 0.4f));
}

FMODSoundSystem::~FMODSoundSystem()
{
	for(FastNameMap<FMODSoundGroup*>::Iterator it = soundGroups.Begin(); it != soundGroups.End(); ++it)
	{
        FMODSoundGroup * soundGroup = it.GetValue();
		SafeRelease(soundGroup);
	}
    soundGroups.Clear();

	FMOD_VERIFY(fmodEventSystem->release());
}

Sound * FMODSoundSystem::CreateSound(const FilePath & fileName, Sound::eType type, const FastName & groupName, bool is3D /* = false */, int32 priority /*  = 128 */)
{
    int32 flags = FMOD_DEFAULT;
    if(is3D)
        flags |= FMOD_3D;
    return FMODSound::CreateWithFlags(fileName, type, groupName, flags, priority);
}

Component * FMODSoundSystem::CreateSoundComponent()
{
    return new FMODSoundComponent();
}

void FMODSoundSystem::LoadAllFEVsRecursive(const DAVA::FilePath & dirPath)
{
    DVASSERT(dirPath.IsDirectoryPathname());

    DAVA::FileList list(dirPath);
    DAVA::int32 entriesCount = list.GetCount();
    for(DAVA::int32 i = 0; i < entriesCount; i++)
    {
        if(list.IsDirectory(i))
        {
            if(!list.IsNavigationDirectory(i))
                LoadAllFEVsRecursive(list.GetPathname(i));
        }
        else
        {
            const DAVA::FilePath & filePath = list.GetPathname(i);

            if(filePath.GetExtension() == ".fev")
                LoadFEV(filePath);
        }
    }
}

void FMODSoundSystem::LoadFEV(const FilePath & filePath)
{
	FMOD_VERIFY(fmodEventSystem->load(filePath.GetAbsolutePathname().c_str(), 0, 0));
}

void FMODSoundSystem::UnloadProjects()
{
    FMOD_VERIFY(fmodEventSystem->unload());
}

FMODSoundSystem * FMODSoundSystem::GetFMODSoundSystem()
{
    FMODSoundSystem * soundSystem = DynamicTypeCheck<FMODSoundSystem*>(SoundSystem::Instance());
    DVASSERT(soundSystem);

    return soundSystem;
}

void FMODSoundSystem::Update(float32 timeElapsed)
{
    SoundSystemInstance::Update(timeElapsed);

    int32 size = removeActiveEventsOnUpdate.size();
    if(size)
    {
        for(int32 i = 0; i < size; i++)
        {
            Vector<FMOD::Event *>::iterator it = activeEvents.begin();
            Vector<FMOD::Event *>::iterator itEnd = activeEvents.end();
            for(; it != itEnd; ++it)
            {
                if((*it) == removeActiveEventsOnUpdate[i])
                {
                    activeEvents.erase(it);
                    break;
                }
            }
        }
        removeActiveEventsOnUpdate.clear();
    }

	fmodEventSystem->update();

    size = soundsToReleaseOnUpdate.size();
    for(int32 i = 0; i < size; i++)
        SafeRelease(soundsToReleaseOnUpdate[i]);
    soundsToReleaseOnUpdate.clear();

    if(callbackOnUpdate.size())
    {
        Map<FMODSoundEvent *, FMODSoundEvent::CallbackType>::iterator mapIt = callbackOnUpdate.begin();
        Map<FMODSoundEvent *, FMODSoundEvent::CallbackType>::iterator endIt = callbackOnUpdate.end();
        for(; mapIt != endIt; ++mapIt)
            mapIt->first->eventDispathcer.PerformEvent(mapIt->second);
        callbackOnUpdate.clear();
    }

    distanceUpdateTime += timeElapsed;
    if(distanceUpdateTime > SOUND_DISTANCE_UPDATE_TIME_SEC)
    {
        distanceUpdateTime = 0.f;

        Vector<FMOD::Event *>::iterator it = activeEvents.begin();
        while(it != activeEvents.end())
        {
            void * userData = 0;
            FMOD_VERIFY((*it)->getUserData(&userData));
            if(!userData) // don't check events from FMODSoundEvent
            {
                FMOD_VECTOR pos;
                FMOD_VERIFY((*it)->get3DAttributes(&pos, 0));
                Vector3 soundPosition(pos.x, pos.y, pos.z);
                if((listenerPosition - soundPosition).SquareLength() > maxDistanceSq)
                {
                    FMOD_VERIFY((*it)->stop());
                    it = activeEvents.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }
}

void FMODSoundSystem::Suspend()
{
#ifdef __DAVAENGINE_ANDROID__
	for(FastNameMap<SoundGroup*>::Iterator it = soundGroups.Begin(); it != soundGroups.End(); ++it)
	{
		SoundGroup * soundGroup = it.GetValue();
		soundGroup->Stop();
	}
#endif
}
    
uint32 FMODSoundSystem::GetMemoryUsageBytes()
{
    uint32 memory = 0;
    
    FMOD_VERIFY(fmodEventSystem->getMemoryInfo(FMOD_MEMBITS_ALL, FMOD_EVENT_MEMBITS_ALL, &memory, 0));
    
    return memory;
}
    
void FMODSoundSystem::Resume()
{
#ifdef __DAVAENGINE_IPHONE__
    FMOD_IPhone_RestoreAudioSession();
#endif
}

void FMODSoundSystem::SetListenerPosition(const Vector3 & position)
{
    if(listenerPosition != position)
    {
        listenerPosition = position;
        FMOD_VECTOR pos = {listenerPosition.x, listenerPosition.y, listenerPosition.z};
        FMOD_VERIFY(fmodEventSystem->set3DListenerAttributes(0, &pos, 0, 0, 0));
    }
}

void FMODSoundSystem::SetListenerOrientation(const Vector3 & forward, const Vector3 & left)
{
	Vector3 forwardNorm = forward;
	forwardNorm.Normalize();
	Vector3 upNorm = forwardNorm.CrossProduct(left);
	upNorm.Normalize();

	FMOD_VECTOR fmodForward = {forwardNorm.x, forwardNorm.y, forwardNorm.z};
	FMOD_VECTOR fmodUp = {upNorm.x, upNorm.y, upNorm.z};
	FMOD_VERIFY(fmodEventSystem->set3DListenerAttributes(0, 0, 0, &fmodForward, &fmodUp));
}

FMODSoundGroup * FMODSoundSystem::GetSoundGroup(const FastName & groupName)
{
	if(soundGroups.IsKey(groupName))
		return soundGroups[groupName];
    else
        return 0;
}

FMODSoundGroup * FMODSoundSystem::CreateSoundGroup(const FastName & groupName)
{
	FMODSoundGroup * group = 0;
	if(soundGroups.IsKey(groupName))
	{
		group = soundGroups[groupName];
	}
    else
    {
        group = new FMODSoundGroup(fmodSystem);
        soundGroups.Insert(groupName, group);
    }

	return group;
}

void FMODSoundSystem::ReleaseOnUpdate(FMODSound * sound)
{
    soundsToReleaseOnUpdate.push_back(sound);
}

void FMODSoundSystem::SetMaxDistance(float32 distance)
{
    maxDistanceSq = distance * distance;
}

float32 FMODSoundSystem::GetMaxDistance()
{
    return pow(maxDistanceSq, .5f);
}

void FMODSoundSystem::GetGroupEventsNamesRecursive(FMOD::EventGroup * group, String & currNamePath, Vector<String> & names)
{
    char * groupName = 0;
    FMOD_VERIFY(group->getInfo(0, &groupName));
    DVASSERT(groupName);
    String currPath = currNamePath + "/" + groupName;

    int32 eventsCount = 0;
    FMOD_VERIFY(group->getNumEvents(&eventsCount));

    for(int32 i = 0; i < eventsCount; i++)
    {
        FMOD::Event * event = 0;
        FMOD_VERIFY(group->getEventByIndex(i, FMOD_EVENT_INFOONLY, &event));
        if(!event)
            continue;

        char * eventName = 0;
        FMOD_VERIFY(event->getInfo(0, &eventName, 0));
        DVASSERT(eventName);

        names.push_back(currPath + "/" + eventName);
    }

    int32 groupsCount = 0;
    FMOD_VERIFY(group->getNumGroups(&groupsCount));
    for(int32 i = 0; i < groupsCount; i++)
    {
        FMOD::EventGroup * childGroup = 0;
        FMOD_VERIFY(group->getGroupByIndex(i, false, &childGroup));
        if(!childGroup)
            continue;

        GetGroupEventsNamesRecursive(childGroup, currPath, names);
    }
}

void FMODSoundSystem::GetAllEventsNames(Vector<String> & names)
{
    names.clear();

    int32 projectsCount = 0;
    FMOD_VERIFY(fmodEventSystem->getNumProjects(&projectsCount));
    for(int32 i = 0; i < projectsCount; i++)
    {
        FMOD::EventProject * project = 0;
        FMOD_VERIFY(fmodEventSystem->getProjectByIndex(i, &project));
        if(!project)
            continue;

        FMOD_EVENT_PROJECTINFO info;
        FMOD_VERIFY(project->getInfo(&info));
        String projectName(info.name);

        int32 groupsCount = 0;        
        FMOD_VERIFY(project->getNumGroups(&groupsCount));
        for(int32 j = 0; j < groupsCount; j++)
        {
            FMOD::EventGroup * group = 0;
            FMOD_VERIFY(project->getGroupByIndex(j, false, &group));
            if(!group)
                continue;

            GetGroupEventsNamesRecursive(group, projectName, names);
        }
    }
}

void FMODSoundSystem::PreloadEventGroupData(const String & groupName)
{
    FMOD::EventGroup * eventGroup = 0;
    FMOD_VERIFY(fmodEventSystem->getGroup(groupName.c_str(), true, &eventGroup));
    if(eventGroup)
        FMOD_VERIFY(eventGroup->loadEventData());
}
    
void FMODSoundSystem::ReleaseEventGroupData(const String & groupName)
{
    FMOD::EventGroup * eventGroup = 0;
    FMOD_VERIFY(fmodEventSystem->getGroup(groupName.c_str(), false, &eventGroup));
    if(eventGroup)
        FMOD_VERIFY(eventGroup->freeEventData());
}
    
void FMODSoundSystem::SetGroupVolume(const FastName & groupName, float32 volume)
{
    FMODSoundGroup * group = GetSoundGroup(groupName);
    if(group)
        group->SetVolume(volume);
}

float32 FMODSoundSystem::GetGroupVolume(const FastName & groupName)
{
    FMODSoundGroup * group = GetSoundGroup(groupName);
    if(group)
        return group->GetVolume();
    else
        return 0.f;
}

void FMODSoundSystem::StopGroup(const FastName & groupName)
{
    FMODSoundGroup * group = GetSoundGroup(groupName);
    if(group)
        return group->Stop();
}

void FMODSoundSystem::SetGlobalComponentsVolume(float32 volume)
{
    globalComponentsVolume = Clamp(volume, 0.f, 1.f);
    
    Vector<FMOD::Event *>::iterator it = activeEvents.begin();
    Vector<FMOD::Event *>::iterator itEnd = activeEvents.end();
    for(; it != itEnd; ++it)
    {
        FMOD_VERIFY((*it)->setVolume(globalComponentsVolume));
    }
}

float32 FMODSoundSystem::GetSoundComponentsVolume()
{
    return globalComponentsVolume;
}

void FMODSoundSystem::AddActiveFMODEvent(FMOD::Event * event)
{
    FMOD_VERIFY(event->setVolume(globalComponentsVolume));
    activeEvents.push_back(event);
}

void FMODSoundSystem::RemoveActiveFMODEvent(FMOD::Event * event)
{
    if(event)
        removeActiveEventsOnUpdate.push_back(event);
}

void FMODSoundSystem::PerformCallbackOnUpdate(FMODSoundEvent * event, FMODSoundEvent::CallbackType type)
{
    callbackOnUpdate[event] = type;
}

};
