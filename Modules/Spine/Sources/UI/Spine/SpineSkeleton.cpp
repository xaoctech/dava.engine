#include "UI/Spine/SpineSkeleton.h"

#include <Logger/Logger.h>

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
    Logger::Debug("[SpineSkeleton::Load] %s, %s", dataPath.GetAbsolutePathname().c_str(), atlasPath.GetAbsolutePathname().c_str());
}

void SpineSkeleton::Update(const float32 timeElapsed)
{

}

void SpineSkeleton::ResetSkeleton()
{
    Logger::Debug("[SpineSkeleton::ResetSkeleton]");
}

BatchDescriptor* SpineSkeleton::GetRenderBatch() const
{
    Logger::Debug("[SpineSkeleton::GetRenderBatch]");
    return nullptr;
}

Vector<String> SpineSkeleton::GetAvailableAnimationsNames() const
{
    Logger::Debug("[SpineSkeleton::GetAvailableAnimationsNames]");
    return Vector<String>();
}

SpineTrackEntry* SpineSkeleton::SetAnimation(int32 trackIndex, const String& name, bool loop)
{
    Logger::Debug("[SpineSkeleton::SetAnimation] %d, %s, %d", trackIndex, name.c_str(), loop);
    return nullptr;
}

SpineTrackEntry* SpineSkeleton::AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay)
{
    Logger::Debug("[SpineSkeleton::AddAnimation] %d, %s, %d, %f", trackIndex, name.c_str(), loop, delay);
    return nullptr;
}

SpineTrackEntry* SpineSkeleton::GetTrack(int32 trackIndex)
{
    Logger::Debug("[SpineSkeleton::GetTrack] %d", trackIndex);
    return nullptr;
}

void SpineSkeleton::SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration)
{
    Logger::Debug("[SpineSkeleton::SetAnimationMix] %s, %s, %f", fromAnimation.c_str(), toAnimation.c_str(), duration);
}

void SpineSkeleton::ClearTracks()
{
    Logger::Debug("[SpineSkeleton::ClearTracks]");
}

void SpineSkeleton::CleatTrack(int32 trackIndex)
{
    Logger::Debug("[SpineSkeleton::CleatTrack] %d", trackIndex);
}

void SpineSkeleton::SetTimeScale(float32 timeScale)
{
    Logger::Debug("[SpineSkeleton::SetTimeScale] %f", timeScale);
}

float32 SpineSkeleton::GetTimeScale() const
{
    Logger::Debug("[SpineSkeleton::GetTimeScale]");
    return 1.f;
}

void SpineSkeleton::SetSkin(const String& skinName)
{
    Logger::Debug("[SpineSkeleton::SetSkin] %s", skinName.c_str());
}

void SpineSkeleton::SetSkin(int32 skinNumber)
{
    Logger::Debug("[SpineSkeleton::SetSkin] %d", skinNumber);
}

int32 SpineSkeleton::GetSkinNumber()
{
    Logger::Debug("[SpineSkeleton::GetSkinNumber]");
    return 0;
}

SpineBone* SpineSkeleton::FindBone(const String& boneName)
{
    Logger::Debug("[SpineSkeleton::FindBone] %s", boneName.c_str());
    return nullptr;
}

}