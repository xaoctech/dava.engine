#include "UI/Spine/SpineSkeleton.h"

namespace DAVA
{

SpineSkeleton::SpineSkeleton()
{
}

SpineSkeleton::SpineSkeleton(const SpineSkeleton& src)
{
}

SpineSkeleton::~SpineSkeleton()
{

}

void SpineSkeleton::Load(const FilePath& dataPath, const FilePath& atlasPath)
{

}

void SpineSkeleton::Update(const float32 timeElapsed)
{

}

void SpineSkeleton::ResetSkeleton()
{

}

BatchDescriptor* SpineSkeleton::GetRenderBatch() const
{
    return nullptr;
}

Vector<String> SpineSkeleton::GetAvailableAnimationsNames() const
{
    return Vector<String>();
}

SpineTrackEntry* SpineSkeleton::SetAnimation(int32 trackIndex, const String& name, bool loop)
{
    return nullptr;
}

SpineTrackEntry* SpineSkeleton::AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay)
{
    return nullptr;
}

SpineTrackEntry* SpineSkeleton::GetTrack(int32 trackIndex)
{
    return nullptr;
}

void SpineSkeleton::SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration)
{

}

void SpineSkeleton::ClearTracks()
{

}

void SpineSkeleton::CleatTrack(int32 trackIndex)
{

}

void SpineSkeleton::SetTimeScale(float32 timeScale)
{

}

float32 SpineSkeleton::GetTimeScale() const
{
    return 1.f;
}

void SpineSkeleton::SetSkin(const String& skinName)
{

}

void SpineSkeleton::SetSkin(int32 skinNumber)
{

}

int32 SpineSkeleton::GetSkinNumber()
{
    return 0;
}

SpineBone* SpineSkeleton::FindBone(const String& boneName)
{
    return nullptr;
}

}