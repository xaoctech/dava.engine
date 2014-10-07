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


#include "Scene3D/AnimationData.h"
#include "Scene3D/SceneNodeAnimationList.h"

namespace DAVA 
{


AnimationData::AnimationData(int32 _keyCount)
{
	keyCount = _keyCount;
	startIdx = 0;
	keys = new SceneNodeAnimationKey[keyCount];
}

AnimationData::~AnimationData()
{
	SafeDeleteArray(keys);
}
	
void AnimationData::SetKey(int32 index, const SceneNodeAnimationKey & key)
{
	keys[index] = key;
}

SceneNodeAnimationKey AnimationData::Interpolate(float32 t)
{
	if (this->keyCount == 1)
	{
		return this->keys[0];
	}
	
	if (t < this->keys[startIdx].time)
	{
		startIdx = 0;
	}
	
	int32 endIdx = 0;
	for (endIdx = startIdx; endIdx < keyCount; ++endIdx)
	{
		if (keys[endIdx].time > t)
		{
			break;
		}
		startIdx = endIdx;
	}
	
	if (endIdx == keyCount)
	{
		startIdx = keyCount - 1;
		endIdx = 0;
	}
	
	SceneNodeAnimationKey & key1 = keys[startIdx];
	SceneNodeAnimationKey & key2 = keys[endIdx];

	float32 tInter;
	if (endIdx > startIdx)
		tInter = (t - key1.time) / (key2.time - key1.time);
	else
		tInter = (t - key1.time) / (duration - key1.time);

	SceneNodeAnimationKey result;
	result.translation.Lerp(key1.translation, key2.translation, tInter);
	result.rotation.Slerp(key1.rotation, key2.rotation, tInter);
	result.scale.Lerp(key1.scale, key2.scale, tInter);

	return result;
}

void AnimationData::SetDuration(float32 _duration)
{
	duration = _duration;
}
	
void AnimationData::SetInvPose(const Matrix4& mat)
{
	invPose = mat;
}
const Matrix4& AnimationData::GetInvPose() const
{
	return invPose;
}
	
AnimationData* AnimationData::Clone() const
{
	AnimationData* copy = new AnimationData(keyCount);

	copy->invPose = invPose;
	copy->duration = duration;
	for (int32 keyIndex = 0; keyIndex < keyCount; ++keyIndex)
	{
		copy->keys[keyIndex] = keys[keyIndex];
	}

	return copy;
}

}
