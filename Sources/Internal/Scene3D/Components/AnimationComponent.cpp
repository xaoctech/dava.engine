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
repeat(true),
frameIndex(0)
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
		archive->SetVariant("animation", VariantType((uint64)animation));
		archive->SetBool("autostart", autoStart);
		archive->SetBool("repeat", repeat);
	}
}

void AnimationComponent::Deserialize(KeyedArchive *archive, SerializationContext *sceneFile)
{
	if(NULL != archive)
	{
		AnimationData* newAnimation = static_cast<AnimationData*>(sceneFile->GetDataBlock(archive->GetVariant("animation")->AsUInt64()));
		if (animation != newAnimation)
		{
			SafeRelease(animation);
			animation = SafeRetain(newAnimation);
		}
		autoStart = archive->GetBool("autostart", true);
		repeat = archive->GetBool("repeat", true);
	}

	Component::Deserialize(archive, sceneFile);
}

void AnimationComponent::GetDataNodes(Set<DataNode*> & dataNodes)
{
	if (animation)
		dataNodes.insert(animation);
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
        animationTransform.Identity();
	}
	time = 0.0f;
}

bool AnimationComponent::GetIsPlaying() const
{
	return isPlaying;
}

};