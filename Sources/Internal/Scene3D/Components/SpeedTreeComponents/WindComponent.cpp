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



#include "Scene3D/Components/SpeedTreeComponents/WindComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA 
{
	REGISTER_CLASS(WindComponent)

WindComponent::WindComponent() :
    windForce(1.f)
{
    
}

WindComponent::~WindComponent()
{
    
}
 
Component * WindComponent::Clone(Entity * toEntity)
{
    WindComponent * component = new WindComponent();
	component->SetEntity(toEntity);
    
	component->windForce = windForce;
    
    return component;
}

void WindComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(archive != 0)
	{
        archive->SetFloat("wc.windForce", windForce);
    }
}
    
void WindComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	if(archive)
	{
        windForce = archive->GetFloat("wc.windForce");
	}

	Component::Deserialize(archive, serializationContext);
}
    
Vector3 WindComponent::GetWindDirection() const
{
    DVASSERT(entity);
    DVASSERT(GetTransformComponent(entity));

    return MultiplyVectorMat3x3(Vector3(1.f, 0.f, 0.f), GetTransformComponent(entity)->GetWorldTransform());
}
    
void WindComponent::SetWindForce(const float32 & force)
{
    windForce = force;
}
    
float32 WindComponent::GetWindForce() const
{
    return windForce;
}
    
};
