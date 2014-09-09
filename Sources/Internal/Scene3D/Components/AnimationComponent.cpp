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
#include "Scene3D/SceneNodeAnimation.h"

namespace DAVA
{

REGISTER_CLASS(AnimationComponent)
    
AnimationComponent::AnimationComponent()
:time(0.0f),
isPlaying(true),
animation(NULL)
{
}
    
AnimationComponent::~AnimationComponent()
{
    SafeRelease(animation);
}

Component * AnimationComponent::Clone(Entity * toEntity)
{
    AnimationComponent * newAnimation = new AnimationComponent();

    newAnimation->originalMatrix = originalMatrix;
    newAnimation->originalTranslate = originalTranslate;
    newAnimation->time = time;
    newAnimation->isPlaying = true;
    newAnimation->animation = animation ? animation->Clone() : NULL;
    return newAnimation;
}



void AnimationComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive)
	{
        archive->SetFloat("duration", animation->duration);
        archive->SetInt32("keyCount", animation->keyCount);
        for (int32 keyIndex = 0; keyIndex < animation->keyCount; ++keyIndex)
        {
            archive->SetFloat(Format("key_%i_time", keyIndex), animation->keys[keyIndex].time);
            archive->SetVector3(Format("key_%i_translation", keyIndex), animation->keys[keyIndex].translation);
            archive->SetVector4(Format("key_%i_rotation", keyIndex), Vector4(animation->keys[keyIndex].rotation.x, animation->keys[keyIndex].rotation.y, animation->keys[keyIndex].rotation.z, animation->keys[keyIndex].rotation.w));
        }
	}
}

void AnimationComponent::Deserialize(KeyedArchive *archive, SerializationContext *sceneFile)
{
	if(NULL != archive)
	{
        const int32 keyCount = archive->GetInt32("keyCount");

        SafeRelease(animation);
        animation = new SceneNodeAnimation(keyCount);

        animation->SetDuration(archive->GetFloat("duration"));

        for (int32 keyIndex = 0; keyIndex < keyCount; ++keyIndex)
        {
            SceneNodeAnimationKey key;

            key.time = archive->GetFloat(Format("key_%i_time", keyIndex));
            key.translation = archive->GetVector3(Format("key_%i_translation", keyIndex));
            Vector4 rotation = archive->GetVector4(Format("key_%i_rotation", keyIndex));
            key.rotation = Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);

            animation->SetKey(keyIndex, key);
        }
// 		localMatrix = archive->GetMatrix4("tc.localMatrix", Matrix4::IDENTITY);
// 		worldMatrix = archive->GetMatrix4("tc.worldMatrix", Matrix4::IDENTITY);
	}

	Component::Deserialize(archive, sceneFile);
}

void AnimationComponent::SetLocalTransform( const Matrix4 & transform )
{
    originalMatrix = transform;
    originalTranslate = originalMatrix.GetTranslationVector();
    originalMatrix.SetTranslationVector(Vector3());
}

void AnimationComponent::SetAnimation(SceneNodeAnimation* _animation)
{
    SafeRetain(_animation);
    SafeRelease(animation);
    animation = _animation;
}

};