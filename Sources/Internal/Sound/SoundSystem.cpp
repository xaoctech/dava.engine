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

#include "Sound/SoundSystem.h"
#include "Sound/SoundChannel.h"
#include "Sound/ALUtils.h"
#include "Sound/SoundInstance.h"
#include "Sound/SoundGroup.h"

#ifdef __DAVAENGINE_IPHONE__
#include "AudioToolbox/AudioServices.h"
#endif


#ifdef __DAVAENGINE_IPHONE__
void interrruptionListenerCallback(void * userData, UInt32 iterruptionState)
{

}
#endif

namespace DAVA
{
    
#ifdef __DAVASOUND_AL__
    ALCcontext * context = NULL;
    ALCdevice * device = NULL;
#endif //#ifdef __DAVASOUND_AL__
    SoundSystem::SoundSystem(int32 _maxChannels)
    :maxChannels(_maxChannels),
        volume(1.f)
    {
#ifdef __DAVAENGINE_IPHONE__
        OSStatus result = AudioSessionInitialize(NULL, NULL, interrruptionListenerCallback, NULL);
        UInt32 category = kAudioSessionCategory_AmbientSound;
        result = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
#endif
        
#ifdef __DAVASOUND_AL__
	device = alcOpenDevice(0);
	if(device)
	{
		context = alcCreateContext(device, 0);
		AL_CHECKERROR();
		AL_VERIFY(alcMakeContextCurrent(context));
		AL_VERIFY(alDistanceModel( AL_INVERSE_DISTANCE_CLAMPED ) );
	}
#endif //#ifdef __DAVASOUND_AL__

#ifdef __DAVAENGINE_ANDROID__    
    SLresult result;
    
    engineObject = NULL;
    engineEngine = NULL;
    outputMixObject = NULL;
    
    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    DVASSERT(SL_RESULT_SUCCESS == result);
    
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    DVASSERT(SL_RESULT_SUCCESS == result);
#else
    
	for(int32 i = 0; i < maxChannels; ++i)
	{
		SoundChannel * ch = new SoundChannel();
		channelsPool.push_back(ch);
	}
#endif //#ifdef __DAVAENGINE_ANDROID__

	groupFX = new SoundGroup();
	groupMusic = new SoundGroup();
}

SoundSystem::~SoundSystem()
{
	SafeDelete(groupMusic);
	SafeDelete(groupFX);

	for(int32 i = 0; i < maxChannels; ++i)
	{
		SoundChannel * ch = channelsPool[i];
		delete(ch);
	}
#ifdef __DAVASOUND_AL__
	alcMakeContextCurrent(0);
	alcDestroyContext(context);
	alcCloseDevice(device);
#endif //#ifdef __DAVASOUND_AL__
    
#ifdef __DAVAENGINE_ANDROID__
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }
    
    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

#endif //#ifdef __DAVAENGINE_ANDROID__
}
    
#ifdef __DAVAENGINE_ANDROID__
SLObjectItf SoundSystem::getEngineObject()
{
    return engineObject;
}

SLEngineItf SoundSystem::getEngineEngine()
{
    return engineEngine;
}
    
SLObjectItf SoundSystem::getOutputMixObject()
{
    return outputMixObject;
}
#endif //#ifdef __DAVAENGINE_ANDROID__
SoundChannel * SoundSystem::FindChannel(int32 priority)
{
	SoundChannel * ch = 0;

	Deque<SoundChannel*>::iterator it;
	Deque<SoundChannel*>::iterator itEnd = channelsPool.end();
	for(it = channelsPool.begin(); it != itEnd; ++it)
	{
		ch = *it;
		if(SoundChannel::STATE_FREE == ch->GetState())
		{
			break;
		}
	}

	if(!ch)
	{
		for(it = channelsPool.begin(); it != itEnd; ++it)
		{
			ch = *it;
			if(ch->GetProirity() < priority)
			{
				ch->Stop();
				break;
			}
		}
	}

	return ch;
}

void SoundSystem::Update()
{
	Deque<SoundChannel*>::iterator it;
	Deque<SoundChannel*>::iterator itEnd = channelsPool.end();
	for(it = channelsPool.begin(); it != itEnd; ++it)
	{
		SoundChannel * ch = *it;
		if(SoundChannel::STATE_FREE != ch->GetState())
		{
			ch->Update();
		}
	}

	List<SoundInstance*>::iterator sit = soundInstances.begin();
	List<SoundInstance*>::iterator sEnd = soundInstances.end();
	while(sit != sEnd)
	{
		if(!(*sit)->Update())
		{
			sit = soundInstances.begin();
			continue;
		}
		++sit;
	}
}

void SoundSystem::AddSoundInstance(SoundInstance * soundInstance)
{
	soundInstances.push_back(soundInstance);
}

void SoundSystem::RemoveSoundInstance(SoundInstance * soundInstance)
{
	soundInstances.remove(soundInstance);
}

void SoundSystem::Suspend()
{
#ifdef __DAVAENGINE_ANDROID__
    groupFX->Suspend();
    groupMusic->Suspend();
#else
	Deque<SoundChannel*>::iterator it;
	Deque<SoundChannel*>::iterator itEnd = channelsPool.end();
	for(it = channelsPool.begin(); it != itEnd; ++it)
	{
		SoundChannel * ch = *it;
		if(SoundChannel::STATE_PLAYING == ch->GetState())
		{
			ch->Pause(true);
		}
	}
#endif //#ifdef __DAVAENGINE_ANDROID__
    
#ifdef __DAVASOUND_AL__
	alcSuspendContext(context);
#endif //#ifdef __DAVASOUND_AL__
}

void SoundSystem::Resume()
{
#ifdef __DAVAENGINE_ANDROID__
    groupFX->Resume();
    groupMusic->Resume();
#endif //#ifdef __DAVAENGINE_ANDROID__
    
#ifdef __DAVASOUND_AL__
	alcProcessContext(context);
	Deque<SoundChannel*>::iterator it;
	Deque<SoundChannel*>::iterator itEnd = channelsPool.end();
	for(it = channelsPool.begin(); it != itEnd; ++it)
	{
		SoundChannel * ch = *it;
		if(SoundChannel::STATE_PAUSED == ch->GetState())
		{
			ch->Pause(false);
		}
	}
#endif //#ifdef __DAVASOUND_AL__
}

void SoundSystem::SetVolume(float32 _volume)
{
	volume = Clamp(_volume, 0.f, 1.f);
#ifdef __DAVASOUND_AL__
	AL_VERIFY(alListenerf(AL_GAIN, volume));
#endif //#ifdef __DAVASOUND_AL__
}

void SoundSystem::SetPosition(const Vector3 & position)
{
#ifdef __DAVASOUND_AL__
	AL_VERIFY(alListener3f(AL_POSITION, position.x, position.y, position.z));
#endif //#ifdef __DAVASOUND_AL__
}

void SoundSystem::SetOrientation(const Vector3 & at, const Vector3 & up)
{
#ifdef __DAVASOUND_AL__
	ALfloat listenerOri[]={at.x, at.y, at.z, up.x, up.y, up.z};
	AL_VERIFY(alListenerfv(AL_ORIENTATION, listenerOri));
#endif //#ifdef __DAVASOUND_AL__
}

float32 SoundSystem::GetVolume()
{
	return volume;
}

SoundGroup		* SoundSystem::GroupFX()
{
	return groupFX;
}

SoundGroup		* SoundSystem::GroupMusic()
{
	return groupMusic;
}



};
