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

#include "Sound/FMODSoundSystem.h"
#include "Sound/FMODSound.h"
#include "Sound/FMODUtils.h"
#include "FileSystem/FileList.h"
#include "Scene3D/Entity.h"

#ifdef __DAVAENGINE_IPHONE__
#include "fmodiphone.h"
#include "musicios.h"
#endif

namespace DAVA
{

FMOD_RESULT F_CALLBACK FMOD_FILE_OPENCALLBACK(const char * name, int unicode, unsigned int * filesize, void ** handle, void ** userdata);
FMOD_RESULT F_CALLBACK FMOD_FILE_READCALLBACK(void * handle, void * buffer, unsigned int sizebytes, unsigned int * bytesread, void * userdata);
FMOD_RESULT F_CALLBACK FMOD_FILE_SEEKCALLBACK(void * handle, unsigned int pos, void * userdata);
FMOD_RESULT F_CALLBACK FMOD_FILE_CLOSECALLBACK(void * handle, void * userdata);


FMODSoundSystem::FMODSoundSystem(int32 maxChannels /* = 64 */) :
maxDistanceSq(150.f * 150.f)
{
	FMOD_VERIFY(FMOD::EventSystem_Create(&fmodEventSystem));
	FMOD_VERIFY(fmodEventSystem->getSystemObject(&fmodSystem));
#ifdef DAVA_FMOD_PROFILE
    FMOD_VERIFY(fmodEventSystem->init(maxChannels, FMOD_INIT_NORMAL | FMOD_INIT_ENABLE_PROFILE, 0));
#else
    FMOD_VERIFY(fmodEventSystem->init(maxChannels, FMOD_INIT_NORMAL, 0));
#endif
    FMOD_VERIFY(fmodSystem->set3DSettings(1.f, 1.f, 0.4f));
    FMOD_VERIFY(fmodSystem->setFileSystem(FMOD_FILE_OPENCALLBACK, FMOD_FILE_CLOSECALLBACK, FMOD_FILE_READCALLBACK, FMOD_FILE_SEEKCALLBACK, 0, 0, -1));
}

FMODSoundSystem::~FMODSoundSystem()
{
	FMOD_VERIFY(fmodEventSystem->release());
}

SoundEvent * FMODSoundSystem::CreateSoundEventByID(const String & eventName, const FastName & groupName)
{
    SoundEvent * event = new FMODSoundEvent(eventName);
    AddSoundEventToGroup(groupName, event);

    //Logger::Debug("[FMODSoundSystem::CreateSoundEventByID] %x %s", event, eventName.c_str());
    
    return event;
}

SoundEvent * FMODSoundSystem::CreateSoundEventFromFile(const FilePath & fileName, const FastName & groupName, uint32 flags /* = SOUND_EVENT_DEFAULT */, int32 priority /* = 128 */)
{
    SoundEvent * event = 0;
    
#ifdef __DAVAENGINE_IPHONE__
    if((flags & SoundEvent::SOUND_EVENT_CREATE_STREAM) && !(flags & SoundEvent::SOUND_EVENT_CREATE_3D))
    {
        MusicIOSSoundEvent * musicEvent = MusicIOSSoundEvent::CreateMusicEvent(fileName);
        if(flags & SoundEvent::SOUND_EVENT_CREATE_LOOP)
            musicEvent->SetLoopCount(-1);
    }
#endif //__DAVAENGINE_IPHONE__
    
    if(!event)
    {
        event = FMODSound::CreateWithFlags(fileName, flags, priority);
    }
    
    AddSoundEventToGroup(groupName, event);
    
    return event;
}

void FMODSoundSystem::LoadFEV(const FilePath & filePath)
{
    FMOD::EventProject * project = 0;
	FMOD_VERIFY(fmodEventSystem->load(filePath.GetFrameworkPath().c_str(), 0, &project));
    
    if(project)
    {
        FMOD_EVENT_PROJECTINFO info;
        FMOD_VERIFY(project->getInfo(&info));
        String projectName(info.name);
        
        int32 groupsCount = 0;
        FMOD_VERIFY(project->getNumGroups(&groupsCount));
        for(int32 i = 0; i < groupsCount; ++i)
        {
            FMOD::EventGroup * group = 0;
            FMOD_VERIFY(project->getGroupByIndex(i, false, &group));
            
            char * buf = 0;
            FMOD_VERIFY(group->getInfo(0, &buf));
            toplevelGroups.push_back(projectName + "/" + buf);
        }
    }
    
}

void FMODSoundSystem::UnloadFMODProjects()
{
    FMOD_VERIFY(fmodEventSystem->unload());
    
    toplevelGroups.clear();
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

    fmodEventSystem->update();

    if(callbackOnUpdate.size())
    {
        MultiMap<FMODSoundEvent *, FMODSoundEvent::SoundEventCallback>::iterator mapIt = callbackOnUpdate.begin();
        MultiMap<FMODSoundEvent *, FMODSoundEvent::SoundEventCallback>::iterator endIt = callbackOnUpdate.end();
        for(; mapIt != endIt; ++mapIt)
            mapIt->first->PerformEvent(mapIt->second);
        callbackOnUpdate.clear();
    }

    int32 size = soundsToReleaseOnUpdate.size();
    if(size)
    {
        for(int32 i = 0; i < size; i++)
            soundsToReleaseOnUpdate[i]->Release();
        soundsToReleaseOnUpdate.clear();
    }
}

void FMODSoundSystem::Suspend()
{

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

void FMODSoundSystem::SetCurrentLocale(const String & langID)
{
    FMOD_VERIFY(fmodEventSystem->setLanguage(langID.c_str()));
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

void FMODSoundSystem::ReleaseOnUpdate(SoundEvent * sound)
{
    soundsToReleaseOnUpdate.push_back(sound);
}

void FMODSoundSystem::SetMaxDistance(float32 distance)
{
    maxDistanceSq = distance * distance;
}

float32 FMODSoundSystem::GetMaxDistanceSquare()
{
    return maxDistanceSq;
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

void FMODSoundSystem::PreloadFMODEventGroupData(const String & groupName)
{
    FMOD::EventGroup * eventGroup = 0;
    FMOD_VERIFY(fmodEventSystem->getGroup(groupName.c_str(), true, &eventGroup));
    if(eventGroup)
        FMOD_VERIFY(eventGroup->loadEventData());
}
    
void FMODSoundSystem::ReleaseFMODEventGroupData(const String & groupName)
{
    FMOD::EventGroup * eventGroup = 0;
    FMOD_VERIFY(fmodEventSystem->getGroup(groupName.c_str(), false, &eventGroup));
    if(eventGroup)
        FMOD_VERIFY(eventGroup->freeEventData());
}
    
void FMODSoundSystem::ReleaseAllEventWaveData()
{
    int32 topCount = toplevelGroups.size();
    for(int32 i = 0; i < topCount; ++i)
        ReleaseFMODEventGroupData(toplevelGroups[i]);
}
    
void FMODSoundSystem::SetGroupVolume(const FastName & groupName, float32 volume)
{
    for(size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup & group = soundGroups[i];
        if(group.name == groupName)
        {
            group.volume = volume;

            Vector<SoundEvent *> & events = group.events;
            for(size_t i = 0; i < events.size(); ++i)
                events[i]->SetVolume(volume);

            break;
        }
    }
}

float32 FMODSoundSystem::GetGroupVolume(const FastName & groupName)
{
    for(size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup & group = soundGroups[i];
        if(group.name == groupName)
            return group.volume;
    }
    return -1.f;
}

void FMODSoundSystem::AddSoundEventToGroup(const FastName & groupName, SoundEvent * event)
{
    for(size_t i = 0; i < soundGroups.size(); ++i)
    {
        SoundGroup & group = soundGroups[i];
        if(group.name == groupName)
        {
            event->SetVolume(group.volume);
            group.events.push_back(event);
            return;
        }
    }

    SoundGroup group;
    group.volume = 1.f;
    group.name = groupName;
    soundGroups.push_back(group);

    AddSoundEventToGroup(groupName, event);
}
    
void FMODSoundSystem::RemoveSoundEventFromGroups(SoundEvent * event)
{
    Vector<SoundGroup>::iterator it = soundGroups.begin();
    while(it != soundGroups.end())
    {
        Vector<SoundEvent *> & events = it->events;
        Vector<SoundEvent *>::iterator itEv = events.begin();
        Vector<SoundEvent *>::const_iterator itEvEnd = events.end();
        while(itEv != itEvEnd)
        {
            if((*itEv) == event)
            {
                it->events.erase(itEv);
                break;
            }

            ++itEv;
        }

        if(!events.size())
            it = soundGroups.erase(it);
        else
            ++it;
    }
}

void FMODSoundSystem::PerformCallbackOnUpdate(FMODSoundEvent * event, FMODSoundEvent::SoundEventCallback type)
{
    callbackOnUpdate.insert(std::pair<FMODSoundEvent *, FMODSoundEvent::SoundEventCallback>(event, type));
}

void FMODSoundSystem::CancelCallbackOnUpdate(FMODSoundEvent * event, FMODSoundEvent::SoundEventCallback type)
{
    if(callbackOnUpdate.size())
    {
        MultiMap<FMODSoundEvent *, FMODSoundEvent::SoundEventCallback>::iterator it = callbackOnUpdate.find(event);
        if(it != callbackOnUpdate.end())
            callbackOnUpdate.erase(it);
    }
}

FMOD_RESULT F_CALLBACK FMOD_FILE_OPENCALLBACK(const char * name, int unicode, unsigned int * filesize, void ** handle, void ** userdata)
{
    File * file = File::Create(FilePath(name), File::OPEN | File::READ);
    if(!file)
        return FMOD_ERR_FILE_NOTFOUND;

    (*filesize) = file->GetSize();
    (*handle) = file;

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_FILE_READCALLBACK(void * handle, void * buffer, unsigned int sizebytes, unsigned int * bytesread, void * userdata)
{
    File * file = (File*)handle;
    (*bytesread) = file->Read(buffer, sizebytes);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_FILE_SEEKCALLBACK(void * handle, unsigned int pos, void * userdata)
{
    File * file = (File*)handle;
    file->Seek(pos, File::SEEK_FROM_START);

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_FILE_CLOSECALLBACK(void * handle, void * userdata)
{
    File * file = (File*)handle;
    SafeRelease(file);

    return FMOD_OK;
}

};
