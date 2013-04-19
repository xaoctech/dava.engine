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
        * Created by Ivan Petrochenko
=====================================================================================*/

#include "Sound/Sound.h"
#include "Sound/SoundWVProvider.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
#include "Sound/SoundOVProvider.h"
#endif //#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

#include "Sound/SoundBuffer.h"
#include "Sound/SoundInstance.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundGroup.h"
#if defined(__DAVAENGINE_IPHONE__)
#include "Sound/MusicIos.h"
#endif //#if defined(__DAVAENGINE_IPHONE__)

#ifdef __DAVAENGINE_ANDROID__
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif //#ifdef __DAVAENGINE_ANDROID__

namespace DAVA
{

Sound * Sound::Create(const FilePath & fileName, eType type, int32 priority)
{
    if(TYPE_STATIC != type && TYPE_STREAMED != type && TYPE_MANAGED != type)
        return 0;
    
    Sound * sound = new Sound(fileName, type, priority);
    if(!sound->Init())
    {
        SafeRelease(sound);
    }
    return sound;
}

Sound	* Sound::CreateFX(const FilePath & fileName, eType type, int32 priority /*= 0*/)
{
    if(TYPE_STATIC != type && TYPE_STREAMED != type && TYPE_MANAGED != type)
        return 0;
    
	Sound * sound = new Sound(fileName, type, priority);
    if(!sound->Init())
    {
        SafeRelease(sound);
    }
    else
    {
        SoundSystem::Instance()->GroupFX()->AddSound(sound);
    }
	return sound;
}

Sound	* Sound::CreateMusic(const FilePath & fileName, eType type, int32 priority /*= 0*/)
{
    if(TYPE_STATIC != type && TYPE_STREAMED != type && TYPE_MANAGED != type)
        return 0;
    
#if defined(__DAVAENGINE_IPHONE__)
    Sound * sound = new MusicIos(fileName);
    if(!sound->Init())
    {
        SafeRelease(sound);
    }
    else
    {
        SoundSystem::Instance()->GroupMusic()->AddSound(sound);
    }
    return sound;
#else
	Sound * sound = new Sound(fileName, type, priority);
    if(!sound->Init())
    {
        SafeRelease(sound);
    }
    else
    {
        SoundSystem::Instance()->GroupMusic()->AddSound(sound);
    }
	return sound;
#endif //#if defined(__DAVAENGINE_IPHONE__)
}

Sound::Sound(const FilePath & _fileName, eType _type, int32 _priority)
:	fileName(_fileName),
	type(_type),
	buffer(0),
	streamBuffer(0),
	priority(_priority),
	provider(0),
	volume(1.f),
	looping(false),
	group(0),
	position(Vector3()),
	ignorePosition(true)
{
#ifdef __DAVAENGINE_ANDROID__
    playerObject = NULL;
    playerPlay = NULL;
    playerBufferQueue = NULL;
    playerVolume = NULL;
    playerSeek = NULL;
#endif //#ifdef __DAVAENGINE_ANDROID__
}

Sound::~Sound()
{
	if(group)
	{
		group->RemoveSound(this);
	}
    
	SafeRelease(buffer);
	SafeRelease(streamBuffer);
	SafeDelete(provider);
}

bool Sound::Init()
{
	int32 strLength = (int32)fileName.ResolvePathname().length();
    if(strLength < 5)
        return false;
    
	if(fileName.IsEqualToExtension(".wav"))
	{
		provider = new SoundWVProvider(fileName);
	}
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
	else if(fileName.IsEqualToExtension(".ogg"))
	{
		provider = new SoundOVProvider(fileName);
	}
#endif //#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

#ifdef __DAVAENGINE_ANDROID__    
    if(TYPE_STATIC == type)
#endif //#ifdef __DAVAENGINE_ANDROID__ 
        if(!provider)
            return false;

    if(TYPE_STATIC == type)
	{
		if(!PrepareStaticBuffer())
            return false;
#ifdef __DAVAENGINE_ANDROID__
        InitBufferQueueAudioPlayer();
#endif //#ifdef __DAVAENGINE_ANDROID__
        
    }
    else if (TYPE_STREAMED == type)
    {
#ifndef __DAVAENGINE_ANDROID__
		if(!provider->Init())
        {
            return false;
        }
        provider->Rewind();
#else
        if(!InitAssetAudioPlayer())
            return false;
#endif //#ifndef __DAVAENGINE_ANDROID__        
    }

#ifdef __DAVAENGINE_ANDROID__
    minVolumeLevel = SL_MILLIBEL_MIN;
    (*playerVolume)->GetMaxVolumeLevel(playerVolume, &maxVolumeLevel);
#endif //#ifdef __DAVAENGINE_ANDROID__  
    return true;
}

#ifdef __DAVAENGINE_ANDROID__
    
bool Sound::InitAssetAudioPlayer()
{
    SLresult result;

    SLEngineItf engineEngine = SoundSystem::Instance()->getEngineEngine();
    SLObjectItf outputMixObject = SoundSystem::Instance()->getOutputMixObject();
    
    // use asset manager to open asset by filename
    AAssetManager* mgr = ((CorePlatformAndroid *)Core::Instance())->GetAssetManager();
    int32 strLength = (int32)fileName.length();
    String filePath = "Data" + fileName.substr(5, strLength - 5);
    AAsset* asset = AAssetManager_open(mgr, filePath.c_str(), AASSET_MODE_UNKNOWN);
    
    if(!asset)
        return false;
    
    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    AAsset_close(asset);
    
    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    //        
    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 3, ids, req);
    DVASSERT(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    DVASSERT(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    DVASSERT(SL_RESULT_SUCCESS == result);

    // get the seek interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_SEEK, &playerSeek);
    DVASSERT(SL_RESULT_SUCCESS == result);

    // get the volume interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &playerVolume);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    return true;
}

bool Sound::InitBufferQueueAudioPlayer()
{
    if (playerObject != NULL)
    {
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playerPlay = NULL;
        playerBufferQueue = NULL;
        playerVolume = NULL;
    }
    
    SLresult result;    
    SLEngineItf engineEngine = SoundSystem::Instance()->getEngineEngine();
    SLObjectItf outputMixObject = SoundSystem::Instance()->getOutputMixObject();

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1,
        (uint32)(provider->GetSampleRate()*1000), (uint32)provider->GetSampleSize(), (uint32)provider->GetSampleSize(),
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    
    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    
    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 3, ids, req);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    // realize the player
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    // get the play interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    DVASSERT(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    DVASSERT(SL_RESULT_SUCCESS == result);

    // get the volume interface
    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &playerVolume);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    return true;
}
#endif //#ifdef __DAVAENGINE_ANDROID__
    
bool Sound::PrepareStaticBuffer()
{
	buffer = SoundBuffer::CreateStatic(fileName);
	if(1 == buffer->GetRetainCount()) 
	{
		if(!provider->Init())
            return false;
        
#ifndef __DAVAENGINE_ANDROID__
		buffer->FullFill(provider);
#endif //#ifdef __DAVAENGINE_ANDROID__
	}
    return true;
}

void Sound::PrepareDynamicBuffers()
{
	buffer = SoundBuffer::CreateStreamed();
	buffer->Fill(provider, provider->GetStreamBufferSize());
	streamBuffer = SoundBuffer::CreateStreamed();
	streamBuffer->Fill(provider, provider->GetStreamBufferSize());
}

void Sound::UpdateDynamicBuffers()
{
	buffer->Release();
	buffer = streamBuffer;
	streamBuffer = SoundBuffer::CreateStreamed();
	bool filled = streamBuffer->Fill(provider, provider->GetStreamBufferSize());
	if(!filled)
	{
		if(looping)
		{
			provider->Rewind();
			streamBuffer->Release();
			streamBuffer = SoundBuffer::CreateStreamed();
			streamBuffer->Fill(provider, provider->GetStreamBufferSize());
		}
		else
		{
			streamBuffer->Release();
			streamBuffer = SoundBuffer::CreateEmpty();
		}
	}
}

SoundInstance * Sound::Play()
{
#ifdef __DAVAENGINE_ANDROID__
    SLresult result;
    
    if(TYPE_STATIC == type)
    {
        result = (*playerBufferQueue)->Clear(playerBufferQueue);
        DVASSERT(SL_RESULT_SUCCESS == result);
        
        buffer->FullFill(provider, playerBufferQueue);
    }
    
    if(TYPE_STREAMED == type)
    {
        result = (*playerSeek)->SetPosition(playerSeek, 0, SL_SEEKMODE_FAST);
        DVASSERT(SL_RESULT_SUCCESS == result);
    }

    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    soundInstances.clear();
    
    SoundInstance * inst = new SoundInstance(this);
    AddSoundInstance(inst);
    
    return soundInstances.front();
#else
    
	if(TYPE_STREAMED == type && soundInstances.size())
	{
		return soundInstances.front();
	}

	SoundChannel * ch = SoundSystem::Instance()->FindChannel(priority);
	if(!ch)
	{
		return 0;
	}

	if(TYPE_STREAMED == type)
	{
		PrepareDynamicBuffers();
	}

	SoundInstance * inst = new SoundInstance();
	inst->buddyChannel = ch;
	AddSoundInstance(inst);
	ch->SetVolume(volume);
	ch->SetPosition(position);
	ch->SetIgnorePosition(ignorePosition);
	ch->Play(this, looping);
	return inst;
#endif //#ifdef __DAVAENGINE_ANDROID__
}

Sound::eType Sound::GetType() const
{
	return type;
}

void Sound::SetVolume(float32 _volume)
{
	volume = Clamp(_volume, 0.f, 1.f);

#ifdef __DAVAENGINE_ANDROID__
    SLresult result;
    SLmillibel sLevel = (SLmillibel)(Clamp(10000 * log10f(volume), (float32)minVolumeLevel, (float32)maxVolumeLevel));
    result = (*playerVolume)->SetVolumeLevel(playerVolume, sLevel);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    return;
#else
	List<SoundInstance*>::iterator sit;
	List<SoundInstance*>::iterator sitEnd = soundInstances.end();
	for(sit = soundInstances.begin(); sit != sitEnd; ++sit)
	{
		(*sit)->SetVolume(volume);
	}
#endif //#ifdef __DAVAENGINE_ANDROID__
}

#ifdef __DAVAENGINE_ANDROID__
SLuint32 Sound::GetPlayState()
{
    SLresult result;
    SLuint32 state = 0;
    
    result = (*playerPlay)->GetPlayState(playerPlay, &state);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    return state;
}
#endif //#ifdef __DAVAENGINE_ANDROID__
    
float32 Sound::GetVolume() const
{
	return volume;
}

void Sound::SetPosition(const Vector3 & _position)
{
	position = _position;
	List<SoundInstance*>::iterator sit;
	List<SoundInstance*>::iterator sitEnd = soundInstances.end();
	for(sit = soundInstances.begin(); sit != sitEnd; ++sit)
	{
		(*sit)->SetPosition(position);
	}
}


void Sound::SetIgnorePosition(bool _ignorePosition)
{
	ignorePosition = _ignorePosition;
	List<SoundInstance*>::iterator sit;
	List<SoundInstance*>::iterator sitEnd = soundInstances.end();
	for(sit = soundInstances.begin(); sit != sitEnd; ++sit)
	{
		(*sit)->SetIgnorePosition(ignorePosition);
	}
}

void Sound::AddSoundInstance(SoundInstance * soundInstance)
{
	soundInstances.push_back(soundInstance);
}

void Sound::RemoveSoundInstance(SoundInstance * soundInstance)
{
	soundInstances.remove(soundInstance);
}

void Sound::SetLooping(bool _looping)
{
	looping = _looping;
    if(TYPE_STREAMED == type)
    {
#ifdef __DAVAENGINE_ANDROID__
        SLresult result;
        
        result = (*playerSeek)->SetLoop(playerSeek, looping, 0, SL_TIME_UNKNOWN);
        DVASSERT(SL_RESULT_SUCCESS == result);
#endif //#ifdef __DAVAENGINE_ANDROID__
    }
}

void Sound::Stop()
{
#ifdef __DAVAENGINE_ANDROID__
    SLresult result;
    
    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    soundInstances.front()->state = SoundInstance::STATE_FORCED_STOPPED;
    
    return;
#endif //#ifdef __DAVAENGINE_ANDROID__
    
	List<SoundInstance*>::iterator sit;
	List<SoundInstance*>::iterator sitEnd = soundInstances.end();
	for(sit = soundInstances.begin(); sit != sitEnd; ++sit)
	{
		(*sit)->Stop();
	}
	soundInstances.clear();

	if(TYPE_STREAMED == type)
	{
		SafeRelease(buffer);
		SafeRelease(streamBuffer);
	}
}

void Sound::SetSoundGroup(SoundGroup * _group)
{
	group = _group;
}

int32 Sound::Release()
{
    if(GetRetainCount() == 1)
    {
        Stop();
    }
    return BaseObject::Release();
}

void Sound::Pause(bool pause)
{
#ifdef __DAVAENGINE_ANDROID__
    SLresult result;
    
    result = (*playerPlay)->SetPlayState(playerPlay, pause ? SL_PLAYSTATE_PAUSED : SL_PLAYSTATE_PLAYING);
    DVASSERT(SL_RESULT_SUCCESS == result);

    return;
#endif //#ifdef __DAVAENGINE_ANDROID__
	List<SoundInstance*>::iterator sit;
	List<SoundInstance*>::iterator sitEnd = soundInstances.end();
	for(sit = soundInstances.begin(); sit != sitEnd; ++sit)
	{
		(*sit)->Pause(pause);
	}
}



};
