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



#include "ImpulseOscillatorComponent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/SpeedTreeSystem/SpeedTreeUpdateSystem.h"

namespace DAVA 
{
	REGISTER_CLASS(ImpulseOscillatorComponent);

ImpulseOscillatorComponent::ImpulseOscillatorComponent(float32 inflDistance /* = 0.f */, float32 force /* = 0.f */) :
    influenceDistance(influenceDistance),
    forceValue(force)
{
    
}

ImpulseOscillatorComponent::~ImpulseOscillatorComponent()
{
    
}
 
Component * ImpulseOscillatorComponent::Clone(Entity * toEntity)
{
    ImpulseOscillatorComponent * component = new ImpulseOscillatorComponent();
	component->SetEntity(toEntity);
    
    component->forceValue = forceValue;
    component->influenceDistance = influenceDistance;
    
    return component;
}

void ImpulseOscillatorComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0)
	{
        archive->SetFloat("ioc.forceValue", forceValue);
        archive->SetFloat("ioc.influenceDistance", influenceDistance);
    }
}
    
void ImpulseOscillatorComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
	{
        forceValue = archive->GetFloat("ioc.forceValue");
        influenceDistance = archive->GetFloat("ioc.influenceDistance");
	}

	Component::Deserialize(archive, serializationContext);
}

void ImpulseOscillatorComponent::TriggerImpulse()
{
    DVASSERT(entity);
    DVASSERT(entity->GetScene());

	entity->GetScene()->speedTreeUpdateSystem->TriggerImpulseOscillator(entity);
}

};
