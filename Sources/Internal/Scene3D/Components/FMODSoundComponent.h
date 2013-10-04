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



#ifndef __DAVAENGINE_SCENE3D_FMOD_SOUND_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_FMOD_SOUND_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/Components/SoundComponent.h"

namespace FMOD
{
class Event;
};

namespace DAVA 
{

class Entity;
class SoundUpdateSystem;
class FMODSoundComponent : public SoundComponent
{
public:
    struct SoundEventParameterInfo
    {
        String name;
        float32 maxValue;
        float32 minValue;
        float32 currentValue;
    };
    
	FMODSoundComponent();
	virtual ~FMODSoundComponent();

	const String & GetEventName();
	void SetEventName(const String & eventName);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

    virtual void Trigger();
    virtual void Stop();
    virtual void SetParameter(const String & paramName, float32 value);
    virtual float32 GetParameter(const String & paramName);

    virtual void SetPosition(const Vector3 & position);

    //FMOD only
    void GetEventParametersInfo(Vector<SoundEventParameterInfo> & paramsInfo);
    
    void KeyOffParameter(const String & paramName);
    
private:
    FMOD::Event * fmodEvent;
	String eventName;
    Vector3 position;

friend class SoundUpdateSystem;
    
public:
	INTROSPECTION_EXTEND(FMODSoundComponent, SoundComponent,
		PROPERTY("eventName", "eventName", GetEventName, SetEventName, I_SAVE | I_VIEW)
		);
};

};

#endif