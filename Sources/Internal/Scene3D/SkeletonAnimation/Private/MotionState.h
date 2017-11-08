#pragma once

#include "Base/AllocatorFactory.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"

#include "MotionTransition.h"

namespace DAVA
{
class BlendTree;
class SkeletonComponent;
class SkeletonPose;
class YamlNode;
struct MotionTransitionInfo;

class MotionState
{
public:
    MotionState() = default;
    ~MotionState();

    void LoadFromYaml(const YamlNode* stateNode);

    void Reset();
    void Update(float32 dTime);
    void EvaluatePose(SkeletonPose* outPose) const;
    void GetRootOffsetDelta(Vector3* offset) const;
    void SyncPhase(const MotionState* withOther);

    bool IsEndReached() const;
    bool IsPhaseEndReached(uint32 phaseIndex) const;
    bool IsMarkerReached(const FastName& marker) const;

    const UnorderedSet<FastName>& GetReachedMarkers() const;

    const FastName& GetID() const;
    const Vector<FastName>& GetBlendTreeParameters() const;

    void BindSkeleton(const SkeletonComponent* skeleton);
    void BindRootNode(const FastName& rootNodeID);

    bool BindParameter(const FastName& parameterID, const float32* param);
    void UnbindParameters();

    void AddTransitionState(const FastName& trigger, MotionState* dstState);
    MotionState* GetTransitionState(const FastName& trigger) const;

protected:
    FastName id;
    BlendTree* blendTree = nullptr;

    Vector<const float32*> boundParams;

    //TODO: *Skinning* restore markers
    //Vector<FastName> markers;
    UnorderedSet<FastName> reachedMarkers;
    UnorderedMap<FastName, MotionState*> transitions;

    Vector3 rootOffset;
    uint32 animationCurrPhaseIndex = 0u;
    uint32 animationPrevPhaseIndex = 0u;
    float32 animationPhase = 0.f;
    bool animationEndReached = false;
};

inline const FastName& MotionState::GetID() const
{
    return id;
}

inline const UnorderedSet<FastName>& MotionState::GetReachedMarkers() const
{
    return reachedMarkers;
}

inline bool MotionState::IsEndReached() const
{
    return animationEndReached;
}

inline bool MotionState::IsPhaseEndReached(uint32 phaseIndex) const
{
    return (animationPrevPhaseIndex == phaseIndex) && (animationCurrPhaseIndex != phaseIndex);
}

inline bool MotionState::IsMarkerReached(const FastName& marker) const
{
    return reachedMarkers.count(marker) > 0;
}

} //ns
