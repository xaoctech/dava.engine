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



#include "Scene3D/Components/SpeedTreeComponents/SpeedTreeComponent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA 
{
	REGISTER_CLASS(SpeedTreeComponent)

SpeedTreeComponent::SpeedTreeComponent() :
    trunkOscillationAmplitude(1.5f),
    leafsOscillationAmplitude(3.f),
    leafsOscillationSpeed(3.f),
    movingOscillationLeafsSpeed(1.f),
    maxAnimatedLOD(0)
{
}

SpeedTreeComponent::~SpeedTreeComponent()
{
    
}
 
Component * SpeedTreeComponent::Clone(Entity * toEntity)
{
    SpeedTreeComponent * component = new SpeedTreeComponent();
	component->SetEntity(toEntity);

    component->trunkOscillationAmplitude = trunkOscillationAmplitude;
    component->leafsOscillationAmplitude = leafsOscillationAmplitude;
    component->leafsOscillationSpeed = leafsOscillationSpeed;
    component->movingOscillationLeafsSpeed = movingOscillationLeafsSpeed;
    component->maxAnimatedLOD = maxAnimatedLOD;
    
    return component;
}

void SpeedTreeComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0)
	{
		archive->SetFloat("stc.trunkOscillationAmplitude", trunkOscillationAmplitude);
		archive->SetFloat("stc.leafsOscillationAmplitude", leafsOscillationAmplitude);
		archive->SetFloat("stc.leafsOscillationSpeed", leafsOscillationSpeed);
        archive->SetFloat("stc.movingOscillationLeafsSpeed", movingOscillationLeafsSpeed);
        archive->SetInt32("stc.maxAnimatedLOD", maxAnimatedLOD);
    }
}
    
void SpeedTreeComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
	{
		trunkOscillationAmplitude = archive->GetFloat("stc.trunkOscillationAmplitude", trunkOscillationAmplitude);
		leafsOscillationAmplitude = archive->GetFloat("stc.leafsOscillationAmplitude", leafsOscillationAmplitude);
		leafsOscillationSpeed = archive->GetFloat("stc.leafsOscillationSpeed", leafsOscillationSpeed);
        movingOscillationLeafsSpeed = archive->GetFloat("stc.movingOscillationLeafsSpeed", movingOscillationLeafsSpeed);
        maxAnimatedLOD = archive->GetInt32("stc.maxAnimatedLOD", maxAnimatedLOD);
	}

	Component::Deserialize(archive, serializationContext);
}

float32 SpeedTreeComponent::GetTrunkOscillationAmplitude() const
{
    return trunkOscillationAmplitude;
}

float32 SpeedTreeComponent::GetLeafsOscillationApmlitude() const
{
    return leafsOscillationAmplitude;
}

float32 SpeedTreeComponent::GetLeafOscillationSpeed() const
{
    return leafsOscillationSpeed;
}

float32 SpeedTreeComponent::GetMovingOscillationLeafsSpeed() const
{
    return movingOscillationLeafsSpeed;
}

int32 SpeedTreeComponent::GetMaxAnimatedLOD() const
{
    return maxAnimatedLOD;
}

void SpeedTreeComponent::SetTrunkOscillationAmplitude(const float32 & amplitude)
{
    trunkOscillationAmplitude = amplitude;
}

void SpeedTreeComponent::SetLeafsOscillationApmlitude(const float32 & amplitude)
{
    leafsOscillationAmplitude = amplitude;
}

void SpeedTreeComponent::SetLeafOscillationSpeed(const float32 & speed)
{
    leafsOscillationSpeed = speed;
}

void SpeedTreeComponent::SetMovingOscillationLeafsSpeed(const float32 & speed)
{
    movingOscillationLeafsSpeed = speed;
}

void SpeedTreeComponent::SetMaxAnimatedLOD(const int32 & lodIndex)
{
    maxAnimatedLOD = lodIndex;
}

};
