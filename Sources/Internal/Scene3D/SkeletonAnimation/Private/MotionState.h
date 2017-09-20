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

    const FastName& GetID() const;
    const Vector<FastName>& GetBlendTreeParameters() const;

    //TODO: *Skinning* think about interface or usage ?
    bool IsPhaseEnd() const;
    bool IsAnimationEnd() const;
    const FastName& GetLastPhaseName() const;

    void BindSkeleton(const SkeletonComponent* skeleton);

    bool BindParameter(const FastName& parameterID, const float32* param);
    void UnbindParameters();

protected:
    FastName id;
    BlendTree* blendTree = nullptr;

    Vector<const float32*> boundParams;
    Vector<FastName> phaseNames;

    Vector3 rootOffset;
    uint32 animationCurrPhaseIndex = 0u;
    uint32 animationPrevPhaseIndex = 0u;
    float32 animationPhase = 0.f;
    bool anyPhaseEnd = false;
};

inline const FastName& MotionState::GetID() const
{
    return id;
}

inline bool MotionState::IsPhaseEnd() const
{
    return anyPhaseEnd;
}

inline bool MotionState::IsAnimationEnd() const
{
    return (anyPhaseEnd && animationCurrPhaseIndex == 0);
}

inline const FastName& MotionState::GetLastPhaseName() const
{
    return phaseNames[animationPrevPhaseIndex];
}

} //ns
