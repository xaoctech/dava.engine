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



#include "Scene3D/Components/SpeedTreeComponent.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA 
{
	REGISTER_CLASS(SpeedTreeComponent)

SpeedTreeComponent::SpeedTreeComponent()
{
}

SpeedTreeComponent::~SpeedTreeComponent()
{
    
}
 
Component * SpeedTreeComponent::Clone(Entity * toEntity)
{
    SpeedTreeComponent * component = new SpeedTreeComponent();
	component->SetEntity(toEntity);

    component->params = params;
    
    return component;
}

void SpeedTreeComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0)
	{
		archive->SetFloat("stc.trunkOscillationAmplitude", params.trunkOscillationAmplitude);
		archive->SetFloat("stc.trunkOscillationSpeed", params.trunkOscillationSpeed);
		archive->SetFloat("stc.leafsOscillationAmplitude", params.leafsOscillationAmplitude);
		archive->SetFloat("stc.leafsOscillationSpeed", params.leafsOscillationSpeed);
		archive->SetFloat("stc.movingOscillationLeafsSpeed", params.movingOscillationLeafsSpeed);
    }
}
    
void SpeedTreeComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
	{
		params.trunkOscillationAmplitude = archive->GetFloat("stc.trunkOscillationAmplitude", params.trunkOscillationAmplitude);
		params.trunkOscillationSpeed = archive->GetFloat("stc.trunkOscillationSpeed", params.trunkOscillationSpeed);
		params.leafsOscillationAmplitude = archive->GetFloat("stc.leafsOscillationAmplitude", params.leafsOscillationAmplitude);
		params.leafsOscillationSpeed = archive->GetFloat("stc.leafsOscillationSpeed", params.leafsOscillationSpeed);
		params.movingOscillationLeafsSpeed = archive->GetFloat("stc.movingOscillationLeafsSpeed", params.movingOscillationLeafsSpeed);
	}

	Component::Deserialize(archive, serializationContext);
}

SpeedTreeComponent::OscillationParams & SpeedTreeComponent::GetOcciliationParameters()
{
    return params;
}
    
};
