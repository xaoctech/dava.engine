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
#include "Scene3D/AnimationData.h"

namespace DAVA
{

REGISTER_CLASS(AnimationComponent)
    
AnimationComponent::AnimationComponent()
:time(0.0f),
isPlaying(false),
animation(NULL),
autoStart(true),
repeat(true)
{
}

AnimationComponent::~AnimationComponent()
{
	SafeRelease(animation);
}

Component * AnimationComponent::Clone(Entity * toEntity)
{
	AnimationComponent * newAnimation = new AnimationComponent();

	newAnimation->time = time;
	newAnimation->isPlaying = false;
	newAnimation->animation = animation ? animation->Clone() : NULL;
	newAnimation->autoStart = autoStart;
	newAnimation->repeat = repeat;
	return newAnimation;
}



void AnimationComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
{
	Component::Serialize(archive, serializationContext);

	if(NULL != archive)
	{
		archive->SetFloat("duration", animation->duration);
		archive->SetInt32("keyCount", animation->keyCount);
		archive->SetMatrix4("invPose", animation->invPose);

		for (int32 keyIndex = 0; keyIndex < animation->keyCount; ++keyIndex)
		{
			archive->SetFloat(Format("key_%i_time", keyIndex), animation->keys[keyIndex].time);
			archive->SetVector3(Format("key_%i_translation", keyIndex), animation->keys[keyIndex].translation);
			archive->SetVector3(Format("key_%i_scale", keyIndex), animation->keys[keyIndex].scale);
			archive->SetVector4(Format("key_%i_rotation", keyIndex), Vector4(animation->keys[keyIndex].rotation.x, animation->keys[keyIndex].rotation.y, animation->keys[keyIndex].rotation.z, animation->keys[keyIndex].rotation.w));
		}
		archive->SetBool("autostart", autoStart);
		archive->SetBool("repeat", repeat);
	}
}

void AnimationComponent::Deserialize(KeyedArchive *archive, SerializationContext *sceneFile)
{
	if(NULL != archive)
	{
		const int32 keyCount = archive->GetInt32("keyCount");

		SafeRelease(animation);
		animation = new AnimationData(keyCount);

		animation->SetDuration(archive->GetFloat("duration"));
		animation->SetInvPose(archive->GetMatrix4("invPose"));

		for (int32 keyIndex = 0; keyIndex < keyCount; ++keyIndex)
		{
			SceneNodeAnimationKey key;

			key.time = archive->GetFloat(Format("key_%i_time", keyIndex));
			key.translation = archive->GetVector3(Format("key_%i_translation", keyIndex));
			key.scale = archive->GetVector3(Format("key_%i_scale", keyIndex));
			Vector4 rotation = archive->GetVector4(Format("key_%i_rotation", keyIndex));
			key.rotation = Quaternion(rotation.x, rotation.y, rotation.z, rotation.w);

			animation->SetKey(keyIndex, key);
		}

		autoStart = archive->GetBool("autostart", true);
		repeat = archive->GetBool("repeat", true);
	}

	Component::Deserialize(archive, sceneFile);
}

void AnimationComponent::SetAnimation(AnimationData* _animation)
{
	if (_animation == animation)
		return

	SafeRelease(animation);
	animation = SafeRetain(_animation);
}

void AnimationComponent::SetIsPlaying( bool value )
{
	isPlaying = value;

	if (!isPlaying)
	{
		GetEntity()->SetLocalTransform(Matrix4::IDENTITY);
	}
	time = 0.0f;
}

bool AnimationComponent::GetIsPlaying() const
{
	return isPlaying;
}

};