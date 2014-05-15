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



#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA 
{
	REGISTER_CLASS(WaveComponent)

WaveComponent::WaveComponent() :
    amplitude(0.f),
    lenght(0.f),
    speed(0.f),
    damping(0.f),
    infDistance(0.f)
{
}

WaveComponent::WaveComponent(float32 _amlitude, float32 _lenght, float32 _speed, float32 _damping, float32 _infDistance) :
    amplitude(_amlitude),
    lenght(_lenght),
    speed(_speed),
    damping(_damping),
    infDistance(_infDistance)
{
}

WaveComponent::~WaveComponent()
{
    
}

Component * WaveComponent::Clone(Entity * toEntity)
{
    WaveComponent * component = new WaveComponent();
	component->SetEntity(toEntity);
    
    component->amplitude = amplitude;
    component->lenght = lenght;
    component->speed = speed;
    component->damping = damping;
    component->infDistance = infDistance;
    
    return component;
}

void WaveComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0)
    {
        archive->SetFloat("wavec.amplitude", amplitude);
        archive->SetFloat("wavec.lenght", lenght);
        archive->SetFloat("wavec.speed", speed);
        archive->SetFloat("wavec.damping", damping);
        archive->SetFloat("wavec.infDistance", infDistance);
    }
}
    
void WaveComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
    {
        amplitude = archive->GetFloat("wavec.amplitude");
        lenght = archive->GetFloat("wavec.lenght");
        speed = archive->GetFloat("wavec.speed");
        damping = archive->GetFloat("wavec.damping");
        infDistance = archive->GetFloat("wavec.infDistance");
	}

	Component::Deserialize(archive, serializationContext);
}

void WaveComponent::Trigger()
{
    Scene * scene = entity->GetScene();
    DVASSERT(scene);
    DVASSERT(scene->waveSystem);

    scene->waveSystem->WaveTriggered(this);
}

};
