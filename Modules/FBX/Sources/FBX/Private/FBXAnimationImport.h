#pragma once

#include "Animation/AnimationTrack.h"

#include "FBXUtils.h"

namespace DAVA
{
class FilePath;

namespace FBXImporterDetails
{
struct FBXAnimationKey
{
    float32 time;
    Vector4 value;
};

struct FBXAnimationChannelData
{
    FBXAnimationChannelData() = default;
    FBXAnimationChannelData(AnimationTrack::eChannelTarget _channel, const Vector<FBXAnimationKey>& _animationKeys);

    AnimationTrack::eChannelTarget channel = AnimationTrack::CHANNEL_TARGET_COUNT;
    Vector<FBXAnimationKey> animationKeys;
};

struct FBXNodeAnimationData
{
    FbxNode* fbxNode = nullptr;
    Vector<FBXAnimationChannelData> animationTrackData;
};

struct FBXAnimationStackData
{
    String name;
    float32 duration = 0.f;
    Vector<FBXNodeAnimationData> nodesAnimations;
};

//////////////////////////////////////////////////////////////////////////

Vector<FBXAnimationStackData> ImportAnimations(FbxScene* fbxScene);
void SaveAnimation(const FBXAnimationStackData& fbxStackAnimationData, const FilePath& filePath);
};
};