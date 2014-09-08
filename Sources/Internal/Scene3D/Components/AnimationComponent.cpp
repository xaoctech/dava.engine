/*==================================================================================
    Copyright (c) 2014, thorin
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



#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{

REGISTER_CLASS(AnimationComponent)
    
AnimationComponent::AnimationComponent()
{

}
    
AnimationComponent::~AnimationComponent()
{
    
}

Component * AnimationComponent::Clone(Entity * toEntity)
{
    AnimationComponent * newAnimation = new AnimationComponent();

    return newAnimation;
}



void AnimationComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive)
	{
// 		archive->SetMatrix4("tc.localMatrix", localMatrix);
// 		archive->SetMatrix4("tc.worldMatrix", worldMatrix);
	}
}

void AnimationComponent::Deserialize(KeyedArchive *archive, SerializationContext *sceneFile)
{
	if(NULL != archive)
	{
// 		localMatrix = archive->GetMatrix4("tc.localMatrix", Matrix4::IDENTITY);
// 		worldMatrix = archive->GetMatrix4("tc.worldMatrix", Matrix4::IDENTITY);
	}

	Component::Deserialize(archive, sceneFile);
}

void AnimationComponent::PlayClip( const String & clipName, bool repeat )
{

}

void AnimationComponent::SetLocalTransform( const Matrix4 & transform )
{
    originalLocalTransform = transform;
}

};